#include "stack_allocator.h"
#include <Windows.h>
#include <atomic>
#include <boost/context/stack_traits.hpp>
#include <boost/core/ignore_unused.hpp>
#include <cassert>
#include <concurrentqueue.h>
#include <logging/log.h>
#include <random>
#include <thr_queue/thread_api.h>

// see
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa366803(v=vs.85).aspx

namespace game_engine {
namespace thr_queue {
std::atomic< double > ewma_stack_size{ 1 };

allocated_stack::allocated_stack( alloc )
{
  sc.size         = stack_props.stack_size;
  bottom_of_stack = (uint8_t*) VirtualAlloc( nullptr, sc.size, MEM_RESERVE, PAGE_READWRITE );
  assert( bottom_of_stack );
  sc.sp = bottom_of_stack + sc.size;

  if ( !bottom_of_stack ) {
    throw std::logic_error(
      "trying to commit pages of an empty stack"
      " (maybe a master coroutine?)" );
  }
  pages_were_committed = commit_pages_prob( );
  if ( pages_were_committed ) {
    pages = (size_t) std::ceil( ewma_stack_size );
  } else {
    pages = 1;
  }
  pages             = std::max( stack_props.min_stack_size_in_pages, pages );
  auto commit_bytes = pages * stack_props.page_size;
  auto ret = VirtualAlloc( (uint8_t*) sc.sp - commit_bytes, commit_bytes, MEM_COMMIT, PAGE_READWRITE );
  boost::ignore_unused( ret );
  assert( ret );
}

allocated_stack::~allocated_stack( )
{
  if ( !bottom_of_stack ) {
    return;
  }
  auto ret = VirtualFree( bottom_of_stack, 0, MEM_RELEASE );
  assert( ret );
}

int
platform::SEH_filter_except_add_page( thr_queue::allocated_stack* stc, _EXCEPTION_POINTERS* ep )
{
  if ( ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  auto op       = ep->ExceptionRecord->ExceptionInformation[ 0 ];
  auto vaddress = (uint8_t*) ep->ExceptionRecord->ExceptionInformation[ 1 ];
  if ( op != 0 && op != 1 ) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  // leave a guard page
  if ( stc->bottom_of_stack + stack_props.page_size < vaddress && vaddress < stc->sc.sp ) {
    auto bottom_of_page =
      (uint8_t*) ( (std::intptr_t) vaddress / stack_props.page_size * stack_props.page_size );
    auto numbytes = (std::intptr_t) stc->sc.sp - (std::intptr_t) bottom_of_page;
    assert( numbytes % stack_props.page_size == 0 );
    auto ret = VirtualAlloc( bottom_of_page, (size_t) numbytes, MEM_COMMIT, PAGE_READWRITE );
    if ( !ret ) {
      return EXCEPTION_CONTINUE_SEARCH;
    } else {
      stc->pages = numbytes / stack_props.page_size;
      return EXCEPTION_CONTINUE_EXECUTION;
    }
  } else {
    return EXCEPTION_CONTINUE_SEARCH;
  }
}

void
platform::allocated_stack::plat_release( )
{
  if ( pages_were_committed ) {
    return;
  }
  auto* up_ptr = static_cast< thr_queue::allocated_stack* >( this );
  if ( !up_ptr->bottom_of_stack ) {
    return;
  }
  const double a = 0.94;
  double ewma;
  double new_ewma;
  do {
    ewma     = ewma_stack_size;
    new_ewma = (double) pages * ( 1 - a ) + a * ewma;
  } while ( !ewma_stack_size.compare_exchange_weak( ewma, new_ewma ) );
  LOG( ) << "New ewma: " << new_ewma;
}

static boost::mutex commit_pages_mt;

bool
platform::allocated_stack::commit_pages_prob( )
{
  boost::lock_guard< boost::mutex > l( commit_pages_mt );
  static std::mt19937 mt{ std::random_device( ).operator( )( ) };
  static std::bernoulli_distribution d( 0.01 );
  return d( mt );
}
}
}
