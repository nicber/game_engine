#include <aio/aio.h>
#include "../thr_queue/global_thr_pool_impl.h"

namespace game_engine {
namespace aio {
void
aio_operation_base::perform_mayblock_aio_platform(perform_helper_base &, thr_queue::coroutine cor)
{
  replace_running_cor_and_jump(helper, std::move(cor));
  assert(fut_fut.ready());
}

namespace platform {
void
perform_helper_impl::plat_about_to_block()
{
  auto *helper_ptr = static_cast<perform_helper_base*>(this);
  if (helper_ptr->caller_coroutine) {
    apc_pending_exec = true;
    QueueUserAPC([](ULONG_PTR ptr) {
      auto &that = *reinterpret_cast<perform_helper_base*>(ptr);
      if (!that.apc_pending_exec) {
        return;
      }
      that.apc_pending_exec = false;
      thr_queue::global_thr_pool.schedule(std::move(that.caller_coroutine.get()), true);
      that.caller_coroutine = boost::none;
    }, thr_queue::this_wthread->get_internals().thr.native_handle(), (ULONG_PTR) (helper_ptr));
  }
}

void
perform_helper_impl::plat_cant_block_anymore()
{
  if (apc_pending_exec) {
    apc_pending_exec = false;
    WaitForSingleObjectEx(INVALID_HANDLE_VALUE, 0, true);
  }
}
}
}
}
