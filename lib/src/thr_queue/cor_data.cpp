#include "cor_data.h"
#include "global_thr_pool_impl.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/lexical_cast.hpp>
#include <logging/log.h>
#include <set>
#include <stdlib.h>
#include <thr_queue/thread_api.h>

namespace game_engine {
namespace thr_queue {
static boost::mutex cor_mt;
static std::set< cor_data* > cor_set;
thread_local exec_ctx* last_jump_from;

static bool
coroutine_debug( )
{
  static bool res = [] {
    auto ptr = getenv( "COROUTINE_DEBUG" );
    int parse_res;
    bool res = false;
    if ( ptr && boost::conversion::try_lexical_convert( ptr, parse_res ) ) {
      if ( parse_res == 1 ) {
        res = true;
        LOG( ) << "enabling coroutine debugging: COROUTINE_DEBUG=" << 1;
      } else {
        res = false;
        LOG( ) << "disabling coroutine debugging: COROUTINE_DEBUG=" << parse_res;
      }
    } else if ( ptr != nullptr ) {
      LOG( ) << "disabling coroutine debugging: COROUTINE_DEBUG=" << ptr;
    } else {
      LOG( ) << "disabling coroutine debugging: COROUTINE_DEBUG is empty";
    }
    return res;
  }( );
  return res;
}

cor_data::cor_data( ) : alloc_stc( )
{
}

cor_data::cor_data( game_engine::thr_queue::functor_ptr func )
  : alloc_stc( game_engine::thr_queue::allocate_stack( ) )
{
  if ( coroutine_debug( ) ) {
    boost::lock_guard< boost::mutex > l( cor_mt );
    auto ret = cor_set.insert( this );
    boost::ignore_unused_variable_warning( ret );
    assert( ret.second );
  }

  auto start_func = []( exec_ctx came_from,
                        game_engine::thr_queue::functor_ptr f,
                        game_engine::thr_queue::allocated_stack* stc_ptr ) {
    boost::ignore_unused( stc_ptr );
    auto actual_start = came_from( nullptr, nullptr );
    assert( std::get< 1 >( actual_start ) == nullptr );
    *last_jump_from = std::move( std::get< 0 >( actual_start ) );
    last_jump_from  = nullptr;
#ifndef _WIN32
    ( *f )( );
#else
    auto func = [ f_ptr = f.get( ), stc_ptr ]
    {
      __try {
        ( *f_ptr )( );
      } __except ( platform::SEH_filter_except_add_page( stc_ptr, GetExceptionInformation( ) ) ) {
        abort( );
      }
    };
    func( );
#endif
    assert( game_engine::thr_queue::master_coroutine->data_ptr->ctx );
    return std::move( game_engine::thr_queue::master_coroutine->data_ptr->ctx );
  };

  boost::context::preallocated palloc( alloc_stc.sc.sp, alloc_stc.sc.size, alloc_stc.sc );
  ctx = exec_ctx( std::allocator_arg, palloc, game_engine::thr_queue::donothing_allocator( ), start_func );
  // we pass the function it has to run and then we come back.
  auto new_ctx = std::get< 0 >( ctx( move( func ), &alloc_stc ) );
  ctx          = std::move( new_ctx );
}

cor_data::~cor_data( )
{
  if ( coroutine_debug( ) ) {
    boost::lock_guard< boost::mutex > l( cor_mt );
    auto ret = cor_set.erase( this );
    boost::ignore_unused( ret );
    assert( ret == 1 );
  }
  auto exectx = std::move( ctx );
  if ( exectx ) {
    LOG( ) << "destroying coroutine that has not finished yet";
  }
  release_stack( std::move( alloc_stc ) );
}
}
}
