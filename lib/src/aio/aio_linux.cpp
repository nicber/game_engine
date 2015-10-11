#include <aio/aio.h>
#include "../thr_queue/global_thr_pool_impl.h"

namespace game_engine {
namespace aio {
namespace platform {
void
perform_helper_impl::plat_about_to_block()
{
  auto *helper_ptr = (perform_helper_base*)this;
  if (helper_ptr->caller_coroutine) {
    thr_queue::global_thr_pool.schedule(std::move(helper_ptr->caller_coroutine.get()),
                                        true);
    helper_ptr->caller_coroutine = boost::none;
  }
}

void
perform_helper_impl::plat_cant_block_anymore()
{

}
}
}
}
