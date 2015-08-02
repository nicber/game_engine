#include "uv_thread.h"

namespace game_engine {
namespace thr_queue {
namespace event {
static void run_loop();

uv_thread::uv_thread()
 :should_stop(false)
 ,global_async(std::unique_ptr<uv_async_t>(new uv_async_t))
 ,thr(run_loop) {}

uv_thread::~uv_thread()
{
  // we have to make sure that we call uv_async_send with an async handler that's
  // already initialized.
  should_stop = true;
  boost::unique_lock<boost::mutex> lock(init_start_requests_mt, boost::defer_lock);
  while (lock.lock(), !async_constructed) {
    lock.unlock();
  }
  assert (async_constructed);
  uv_async_send(global_async.get());
  lock.unlock();
  thr.join();
}

uv_thread &get_uv_thr() {
  static uv_thread thr;
  return thr;
}

void uv_thr_async_init(uv_async_t *async, uv_async_cb f_ptr) {
  auto fut = uv_thr_sync_do([&] {
    uv_async_init(uv_default_loop(), async, f_ptr);
  });
  fut.get();
}

void process_init_start_requests_or_stop(uv_async_t *) {
  auto &thr = get_uv_thr();

  boost::unique_lock<boost::mutex> lock(thr.init_start_requests_mt,
                                    boost::defer_lock);

  while (lock.lock(), thr.init_start_requests.size()) {
    auto q = std::move(thr.init_start_requests);
    lock.unlock();
    for (auto &f : q) {
      (*f)();
    }
  }

  if (thr.should_stop) {
    uv_stop(uv_default_loop());
    return;
  }
}

void run_loop() {
  auto &thr = get_uv_thr();
  assert(thr.global_async);
  boost::unique_lock<boost::mutex> lock(thr.init_start_requests_mt);
  uv_async_init(uv_default_loop(),
                thr.global_async.get(),
                process_init_start_requests_or_stop);
  thr.async_constructed = true;
  lock.unlock();

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
}
}
}
