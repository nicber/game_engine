#include "global_thr_pool_impl.h"

#include <algorithm>
#include <iterator>

namespace game_engine {
namespace thr_queue {
worker_thread::worker_thread(work_type_data &dat)
    : data(dat), should_stop(false), thr([this] { loop(); }) {}

void worker_thread::loop() {
  coroutine master_cor;
  std::function<void()> after_yield_f;
  master_coroutine = &master_cor;
  after_yield = &after_yield_f;

  boost::unique_lock<boost::mutex> lock_data(data.mt);
  auto while_cond = [&] {
    if (!lock_data.owns_lock()) {
      lock_data.lock();
    }

    return data.work_queue.size() || data.work_queue_prio.size() || should_stop;
  };

  do {
    auto w_it = std::find(
      data.waiting_threads.begin(), data.waiting_threads.end(), this);
    if (w_it != data.waiting_threads.end()) {
      data.waiting_threads.erase(w_it);
    }

    while (while_cond()) {
      if (should_stop) {
        // we inform another thread that we won't be
        // able to do our assigned
        // work.
        if (data.waiting_threads.size()) {
          data.waiting_threads[0]->cv.notify_one();
        }

        goto finish;
      }

      std::deque<coroutine> *work_queue = nullptr;
      if (data.work_queue_prio.size()) {
        work_queue = &data.work_queue_prio;
      } else {
        work_queue = &data.work_queue;
      }

      assert(work_queue->size());
      std::deque<coroutine> work_to_do;
      auto number_jobs_take = work_queue->size() / 10;
      number_jobs_take = std::max(decltype(number_jobs_take)(1), number_jobs_take);
      //printf("stole %d coroutines\n", number_jobs_take);
      auto end_iter = work_queue->begin() + number_jobs_take;
      std::move(work_queue->begin(), end_iter, std::back_inserter(work_to_do));
      work_queue->erase(work_queue->begin(), end_iter);
      lock_data.unlock();

      for (auto& cor : work_to_do) {
        running_coroutine_or_yielded_from = &cor;
        cor.switch_to_from(*master_coroutine);
        if (*after_yield) {
          (*after_yield)();
          *after_yield = std::function<void()>();
        }
        running_coroutine_or_yielded_from = master_coroutine;
      }
    }
    // lock_data is guaranteed to be locked now.

    assert(data.work_queue.size() == 0);
    assert(data.work_queue_prio.size() == 0);

    data.waiting_threads.push_front(this);
  } while (cv.wait(lock_data), true);
  finish:
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
  auto hardware_concurrency = boost::thread::hardware_concurrency();
  auto c_cpu_threads = std::max(1u, hardware_concurrency - 1);
  auto c_io_threads = 1u;

  {
    boost::lock_guard<boost::mutex> lock(io_threads_mt);
    for (size_t i = 0; i < c_io_threads; ++i) {
      io_threads.emplace_back(io_data);
    }
  }

  {
    boost::lock_guard<boost::mutex> lock(cpu_threads_mt);
    for (size_t i = 0; i < c_cpu_threads; ++i) {
      cpu_threads.emplace_back(cpu_data);
    }
  }
}

global_thread_pool::~global_thread_pool() {
  boost::lock_guard<boost::mutex> cpu_lock(cpu_data.mt);
  boost::lock_guard<boost::mutex> io_lock(io_data.mt);

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
  schedule(&cor, &cor + 1, first);
}

void global_thread_pool::yield() {
  assert(running_coroutine_or_yielded_from != master_coroutine &&
         "we can't yield from the master_coroutine");
  master_coroutine->switch_to_from(*running_coroutine_or_yielded_from);
}

thread_local std::function<void()> *after_yield = nullptr;
thread_local coroutine *master_coroutine = nullptr;
thread_local coroutine *running_coroutine_or_yielded_from = nullptr;
global_thread_pool global_thr_pool;
}
}
