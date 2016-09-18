#include "stack_allocator.h"
#include "thr_queue/thread_api.h"
#include <concurrentqueue.h>
#include <logging/log.h>
#include <random>

namespace game_engine {
namespace thr_queue {
const stack_props_t stack_props;
moodycamel::ConcurrentQueue< allocated_stack > used_stacks;
constexpr auto used_stack_lifetime = std::chrono::seconds( 10 );

allocated_stack
allocate_stack( )
{
  auto now = std::chrono::system_clock::now( );
  allocated_stack deqd;
  while ( used_stacks.try_dequeue( deqd ) ) {
    if ( ( now - deqd.last_release ) < used_stack_lifetime ) {
      break;
    }
  }
  if ( deqd.bottom_of_stack ) {
    return deqd;
  } else {
    return allocated_stack( allocated_stack::alloc( ) );
  }
}

void
release_stack( allocated_stack stack )
{
  stack.release( );
  if ( stack.bottom_of_stack ) {
    used_stacks.enqueue( std::move( stack ) );
  }
}

allocated_stack::allocated_stack( allocated_stack&& other )
  : last_release( std::move( other.last_release ) ), sc( other.sc ), bottom_of_stack( other.bottom_of_stack )
{
  other.sc.sp           = nullptr;
  other.sc.size         = 0;
  other.bottom_of_stack = 0;
}

allocated_stack::allocated_stack( ) : bottom_of_stack( nullptr )
{
  sc.sp   = nullptr;
  sc.size = 0;
}

allocated_stack&
allocated_stack::operator=( allocated_stack rhs )
{
  using std::swap;
  swap( last_release, rhs.last_release );
  swap( bottom_of_stack, rhs.bottom_of_stack );
  sc          = rhs.sc;
  rhs.sc.sp   = 0;
  rhs.sc.size = 0;
  return *this;
}

void
allocated_stack::release( )
{
  plat_release( );
  last_release = std::chrono::system_clock::now( );
}
}
}
