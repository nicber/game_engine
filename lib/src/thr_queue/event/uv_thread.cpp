#pragma once

#include <cassert>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include "uv_thread.h"

namespace game_engine {
namespace thr_queue {
namespace event {

static void run_loop();
static std::thread thr(run_loop);
static std::unique_ptr<uv_async_t> global_async = nullptr;
static std::deque<std::function<void()>> init_start_requests;
static std::mutex init_start_requests_mt;

void uv_thr_init(uv_async_t *async, uv_async_cb *f_ptr) {
  assert(global_async);

  std::mutex mt;
  std::condition_variable cv;
  std::lock(init_start_requests_mt, mt);
  std::unique_lock<std::mutex> mt_lock(mt, std::adopt_lock);

  {
    std::lock_guard<std::mutex> req_lock(init_start_requests_mt,
                                         std::adopt_lock);

    init_start_requests.emplace_back([=, &mt, &cv] {
      uv_async_init(uv_default_loop(), async, f_ptr);
      // we make sure that the requesting thread has started waiting.
      std::unique_lock<std::mutex> mt_lock(mt);
      cv.notify_all();
    });
  }

  uv_async_send(global_async.get());

  cv.wait(mt_lock);
}

void process_init_start_requests(uv_async_t *, int) {
  std::unique_lock<std::mutex> lock(init_start_requests_mt, std::defer_lock);

  while (lock.lock(), init_start_requests.size()) {
    auto q = std::move(init_start_requests);
    lock.unlock();
    for (auto &f : q) {
      f();
    }
  }
}

void run_loop() {
  assert(!global_async);
  global_async = std::unique_ptr<uv_async_t>(new uv_async_t);
  uv_async_init(
      uv_default_loop(), global_async.get(), process_init_start_requests);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
}
}
}
