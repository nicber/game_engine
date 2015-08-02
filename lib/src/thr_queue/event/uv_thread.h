#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <thr_queue/functor.h> 
#include <thr_queue/thread_api.h> 
#include <uv.h>

namespace game_engine {
namespace thr_queue {
namespace event {
struct uv_thread {
  uv_thread();

  ~uv_thread();

  std::atomic<bool> should_stop;
  bool async_constructed = false;
  std::unique_ptr<uv_async_t> global_async = nullptr;
  std::deque<std::unique_ptr<functor>> init_start_requests;
  boost::mutex init_start_requests_mt;
  boost::thread thr;
};

uv_thread &get_uv_thr();

template <typename F>
boost::future<void> uv_thr_sync_do(F func);

void uv_thr_async_init(uv_async_t *async, uv_async_cb f_ptr);
}
}
}

#include "uv_thread.inl"
