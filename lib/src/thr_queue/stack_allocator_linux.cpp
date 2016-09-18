#include "stack_allocator.h"
#include "global_thr_pool_impl.h"
#include <logging/log.h>
#include <sys/mman.h>

namespace game_engine {
namespace thr_queue {
allocated_stack::allocated_stack( alloc )
{
  sc.size         = stack_props.stack_size;
  bottom_of_stack = (uint8_t*) mmap( nullptr,
                                     sc.size,
                                     PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_STACK,
                                     -1,
                                     0 );
  if ( bottom_of_stack == MAP_FAILED ) {
    LOG( ) << "mmap failed:" << strerror( errno );
    throw std::system_error( errno, std::system_category( ) );
  }
  sc.sp    = bottom_of_stack + sc.size;
  auto ret = mprotect( bottom_of_stack, 1, PROT_NONE ); // guard page
  assert( ret == 0 );
}

allocated_stack::~allocated_stack( )
{
  if ( !bottom_of_stack ) {
    return;
  }
  auto ret = munmap( bottom_of_stack, sc.size );
  if ( ret != 0 ) {
    LOG( ) << "munmap failed: " << strerror( errno );
    throw std::system_error( errno, std::system_category( ) );
  }
}

void
platform::allocated_stack::plat_release( )
{
}
}
}
