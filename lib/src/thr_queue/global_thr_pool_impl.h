#include "thr_queue/coroutine.h"
#include "thr_queue/global_thr_pool.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace game_engine {
namespace thr_queue {

class worker_thread;
extern thread_local std::function<void()> after_yield;
extern thread_local std::unique_ptr<coroutine> master_coroutine;
extern thread_local coroutine *running_coroutine_or_yielded_from;

struct work_type_data {
  std::mutex mt;
  std::deque<coroutine> work_queue;
  std::deque<coroutine> work_queue_prio;
  std::deque<worker_thread *> waiting_threads;
};

class worker_thread {
public:
  worker_thread(work_type_data &dat)
      : data(dat), should_stop(false), thr(&worker_thread::loop, this) {}

  void loop() {
    master_coroutine =
        std::unique_ptr<coroutine>(new coroutine(coroutine_type::master));

    std::unique_lock<std::mutex> lock(mt);
    do {
      std::unique_lock<std::mutex> lock_data(data.mt, std::defer_lock);

      while (lock_data.lock(),
             data.work_queue.size() || data.work_queue_prio.size() ||
                 should_stop) {
        if (data.waiting_threads.size() &&
            data.waiting_threads.front() == this) {
          data.waiting_threads.pop_front();
        }

        if (should_stop) {
          // we inform another thread that we won't be
          // able to do our assigned
          // work.
          if (data.waiting_threads.size()) {
            data.waiting_threads.front()->notify_work_available();
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

        try {
          running_coroutine_or_yielded_from = &cor;
          cor.switch_to_from(*master_coroutine);
          if (after_yield) {
            after_yield();
            after_yield = std::function<void()>();
          }
          running_coroutine_or_yielded_from = master_coroutine.get();
        }
        catch (...) {
          // TODO log something
        }
      }
      // lock_data is guaranteed to be locked now.

      data.waiting_threads.push_front(this);
    } while (cv.wait(lock), true);
    assert(should_stop);
  }

  void notify_work_available() {
    std::lock_guard<std::mutex> lock(mt);
    cv.notify_all();
  }

  ~worker_thread() {
    should_stop = true;
    cv.notify_all();
    thr.join();
  }

private:
  std::mutex mt;
  std::condition_variable cv;
  work_type_data &data;
  std::atomic<bool> should_stop;
  std::thread thr;
};

class global_thread_pool {
public:
  global_thread_pool() {
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

  void schedule(coroutine cor, bool first) {
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

    std::unique_lock<std::mutex> lock_thr_mt;
    std::unique_lock<std::mutex> lock_wdata;
    if (thr_mt) {
      std::lock(*thr_mt, wdata->mt);
      lock_thr_mt = std::unique_lock<std::mutex>(*thr_mt, std::adopt_lock);
      lock_wdata = std::unique_lock<std::mutex>(wdata->mt, std::adopt_lock);
    } else {
      lock_wdata = std::unique_lock<std::mutex>(wdata->mt);
    }

    if (!first) {
      auto it =
          std::lower_bound(wdata->work_queue.begin(),
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
      auto *thr_notif = wdata->waiting_threads.front();
      lock_wdata.unlock(); // avoid a deadlock.
      thr_notif->notify_work_available();
    }

    return;
  }

  void yield() {
    assert(running_coroutine_or_yielded_from != master_coroutine.get() &&
           "we can't yield from the master_coroutine");
    master_coroutine->switch_to_from(*running_coroutine_or_yielded_from);
  }

  template <typename F>
  void yield(F func) {
    after_yield = std::function<void()>(std::move(func));
    yield();
  }

private:
  std::mutex cpu_threads_mt;
  std::list<worker_thread> cpu_threads;

  std::mutex io_threads_mt;
  std::list<worker_thread> io_threads;
  const unsigned int max_io_threads = 2;

  work_type_data io_data;
  work_type_data cpu_data;
};

extern global_thread_pool global_thr_pool;
}
}
