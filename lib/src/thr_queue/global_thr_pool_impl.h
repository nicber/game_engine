#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include "thr_queue/coroutine.h"

namespace game_engine {
namespace thr_queue {
class worker_thread;

struct work_type_data {
  std::mutex mt;
  std::deque<coroutine> work_queue;
  std::deque<coroutine> work_queue_prio;
  std::deque<worker_thread *> waiting_threads;
};

class worker_thread {
public:
  worker_thread(work_type_data &dat);
  void loop();
  void tell_stop();
  bool try_notify_work_available();
  ~worker_thread();

private:
  friend class global_thread_pool;
  std::condition_variable cv;
  work_type_data &data;
  std::atomic<bool> should_stop;
  std::thread thr;
};

class global_thread_pool {
public:
  global_thread_pool();
  ~global_thread_pool();

  void notify_one_thread(std::unique_lock<std::mutex> &lock_data,
                         std::deque<worker_thread *> &waiting_threads);
  void schedule(coroutine cor, bool first);
  void yield();

  template <typename F>
  void yield(F func);

private:
  work_type_data io_data;
  work_type_data cpu_data;

  std::mutex cpu_threads_mt;
  std::list<worker_thread> cpu_threads;

  std::mutex io_threads_mt;
  std::list<worker_thread> io_threads;
  const unsigned int max_io_threads = 2;
};

extern global_thread_pool global_thr_pool;

extern thread_local std::function<void()> after_yield;
extern thread_local std::unique_ptr<coroutine> master_coroutine;
extern thread_local coroutine *running_coroutine_or_yielded_from;
}
}

#include "global_thr_pool_impl.inl"
