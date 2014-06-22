#include "global_thr_pool_impl.h"

#include <algorithm>

namespace game_engine {
namespace thr_queue {
worker_thread::worker_thread(work_type_data &dat)
    : data(dat), should_stop(false), thr(&worker_thread::loop, this) {}

void worker_thread::loop() {
  master_coroutine =
      std::unique_ptr<coroutine>(new coroutine(coroutine_type::master));

  std::unique_lock<std::mutex> lock_data(data.mt);
  auto while_cond = [&] {
    if (!lock_data.owns_lock()) {
      lock_data.lock();
    }

    return data.work_queue.size() || data.work_queue_prio.size() || should_stop;
  };

  do {
    while (while_cond()) {
      auto w_it = std::find(
          data.waiting_threads.begin(), data.waiting_threads.end(), this);
      if (w_it != data.waiting_threads.end()) {
        data.waiting_threads.erase(w_it);
      }

      if (should_stop) {
        // we inform another thread that we won't be
        // able to do our assigned
        // work.
        if (data.waiting_threads.size()) {
          data.waiting_threads[0]->cv.notify_one();
        }

        return;
      }

      std::deque<coroutine> *work_queue = nullptr;
      if (data.work_queue_prio.size()) {
        work_queue = &data.work_queue_prio;
      } else {
        work_queue = &data.work_queue;
      }

      assert(work_queue->size());
      auto cor = std::move(work_queue->front());
      work_queue->pop_front();
      lock_data.unlock();

      running_coroutine_or_yielded_from = &cor;
      cor.switch_to_from(*master_coroutine);
      if (after_yield) {
        after_yield();
        after_yield = std::function<void()>();
      }
      running_coroutine_or_yielded_from = master_coroutine.get();
    }
    // lock_data is guaranteed to be locked now.

    assert(data.work_queue.size() == 0);
    assert(data.work_queue_prio.size() == 0);

    data.waiting_threads.push_front(this);
  } while (cv.wait(lock_data), true);
  assert(should_stop);
}

void worker_thread::tell_stop() {
  should_stop = true;
  cv.notify_one();
}

worker_thread::~worker_thread() {
  assert(should_stop);
  thr.join();
}

global_thread_pool::global_thread_pool() {
  auto hardware_concurrency = std::thread::hardware_concurrency();
  auto c_cpu_threads = std::max(1u, hardware_concurrency - 1);
  auto c_io_threads = 1u;

  {
    std::lock_guard<std::mutex> lock(io_threads_mt);
    for (size_t i = 0; i < c_io_threads; ++i) {
      io_threads.emplace_back(io_data);
    }
  }

  {
    std::lock_guard<std::mutex> lock(cpu_threads_mt);
    for (size_t i = 0; i < c_cpu_threads; ++i) {
      cpu_threads.emplace_back(cpu_data);
    }
  }
}

global_thread_pool::~global_thread_pool() {
  std::lock_guard<std::mutex> cpu_lock(cpu_data.mt);
  std::lock_guard<std::mutex> io_lock(io_data.mt);

  cpu_data.waiting_threads.clear();
  cpu_data.work_queue.clear();
  cpu_data.work_queue_prio.clear();

  io_data.waiting_threads.clear();
  io_data.work_queue.clear();
  io_data.work_queue_prio.clear();

  for (auto &thr : cpu_threads) {
    thr.tell_stop();
  }
  for (auto &thr : io_threads) {
    thr.tell_stop();
  }
}

void global_thread_pool::schedule(coroutine cor, bool first) {
  bool should_try_add = false;
  work_type_data *wdata = nullptr;
  std::list<worker_thread> *threads = nullptr;
  std::mutex *thr_mt = nullptr;

  if (cor.type() == coroutine_type::io) {
    should_try_add = true;
    wdata = &io_data;
    threads = &io_threads;
    thr_mt = &io_threads_mt;
  } else {
    wdata = &cpu_data;
    threads = nullptr; // we never try to add more cpu threads
  }

  std::unique_lock<std::mutex> lock_wdata(wdata->mt);
  std::unique_lock<std::mutex> lock_thr_mt;
  if (thr_mt) {
    lock_thr_mt = std::unique_lock<std::mutex>(*thr_mt);
  }

  if (!first) {
    auto it = std::lower_bound(wdata->work_queue.begin(),
                               wdata->work_queue.end(),
                               cor,
                               [](const coroutine &lhs, const coroutine &rhs) {
      return lhs.creation_time() < rhs.creation_time();
    });
    wdata->work_queue.insert(it, std::move(cor));
  } else {
    wdata->work_queue_prio.emplace_back(std::move(cor));
  }

  if (should_try_add && wdata->waiting_threads.size() == 0 &&
      threads->size() < max_io_threads) {
    threads->emplace_back(io_data);
  }

  if (wdata->waiting_threads.size()) {
    wdata->waiting_threads[0]->cv.notify_one();
  }

  return;
}

void global_thread_pool::yield() {
  assert(running_coroutine_or_yielded_from != master_coroutine.get() &&
         "we can't yield from the master_coroutine");
  master_coroutine->switch_to_from(*running_coroutine_or_yielded_from);
}

thread_local std::function<void()> after_yield;
thread_local std::unique_ptr<coroutine> master_coroutine;
thread_local coroutine *running_coroutine_or_yielded_from = nullptr;
global_thread_pool global_thr_pool;
}
}
