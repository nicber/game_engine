#include "../thr_queue/global_thr_pool_impl.h"
#include <aio/aio.h>

namespace game_engine {
namespace aio {
void
aio_operation_base::perform_mayblock_aio_platform( perform_helper_base&, thr_queue::coroutine cor )
{
  LOG( ) << "Scheduling cor " << boost::this_thread::get_id( );
  assert( thr_queue::this_wthread );
  cor.set_forbidden_thread( thr_queue::this_wthread );
  thr_queue::global_thr_pool.schedule( std::move( cor ), true );
  LOG( ) << "Waiting for fut_fut";
}

namespace platform {
void
perform_helper_impl::plat_about_to_block( )
{
}

void
perform_helper_impl::plat_cant_block_anymore( )
{
}
}
}
}
