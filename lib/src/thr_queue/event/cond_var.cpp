#include <algorithm>
#include "../global_thr_pool_impl.h"
#include <iterator>
#include "thr_queue/coroutine.h"
#include "thr_queue/event/cond_var.h"
#include <uv.h>
#include "uv_thread.h"

namespace game_engine {
namespace thr_queue {

static std::mutex cond_var_global_mt;
static uv_async_t *cond_var_async;
static std::deque<coroutine> cor_queue;

static void cond_var_cb(uv_async_t *, int) {
  std::deque<coroutine> queue;
  std::unique_lock<std::mutex> lock(cond_var_global_mt, std::defer_lock);

  while (lock.lock(), cor_queue.size()) {
    queue = std::move(cor_queue);
    lock.unlock();

    for (auto &cor : queue) {
      global_thr_pool.schedule(std::move(cor), false);
    }
  }
}

namespace event {
void condition_variable::wait() {
  std::unique_lock<std::mutex> global_lock(cond_var_global_mt);

  if (!cond_var_async) {
    cond_var_async = new uv_async_t;
    uv_thr_init(cond_var_async, cond_var_cb);
  }

  global_thr_pool.yield([&] {
    // even though this code will be run by another coroutine, it'll still
    // be run by the same thread as this coroutine, so no problems should
    // arise when unlocking the mutex.
    // also, running_coroutine_or_yielded from still points to this coroutine.

    {
      std::lock_guard<std::mutex> cond_var_lock(mt);
      waiting_cors.emplace_back(std::move(*running_coroutine_or_yielded_from));
    }

    global_lock.unlock();
    uv_async_send(cond_var_async);
  });
}

void condition_variable::notify() {
  std::lock(mt, cond_var_global_mt);
  std::lock_guard<std::mutex> mt_lock(mt, std::adopt_lock);
  std::lock_guard<std::mutex> global_lock(cond_var_global_mt, std::adopt_lock);

  std::move(
      waiting_cors.begin(), waiting_cors.end(), std::back_inserter(cor_queue));

  uv_async_send(cond_var_async);
}
}
}
}
