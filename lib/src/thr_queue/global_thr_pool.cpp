#include "thr_queue/global_thr_pool.h"

#include <atomic>
#include <list>
#include <memory>
#include <vector>

namespace game_engine {
namespace thr_queue {
class worker_thread {
public:
  worker_thread() : should_stop(false), thr(&worker_thread::loop, this) {}

  worker_thread(thread_pool &thr_pool)
      : work_pool(&thr_pool), should_stop(false),
        thr(&worker_thread::loop, this) {}

  bool try_to_join(thread_pool &thr_pool) {
    std::unique_lock<std::mutex> lock(work_mt, std::defer_lock);

    if (lock.try_lock() && running) {
      work_pool = &thr_pool;
      lock.unlock();
      cv.notify_all();
      return true;
    }

    return false;
  }

  void loop() {
    while (true) {
      std::unique_lock<std::mutex> lock(work_mt);
      running = true;
      while (!should_stop &&
             (work_pool || cv.wait_for(lock, std::chrono::seconds(1), [&]() {
                             return work_pool || should_stop;
                           })) &&
             !should_stop) {
        work_pool->participate(kickable::yes);
        work_pool = nullptr;
      }

      running = false;
    }
  }

  ~worker_thread() {
    should_stop = true;
    cv.notify_all();
    thr.detach();
  }

private:
  std::condition_variable cv;
  std::mutex work_mt;
  thread_pool *work_pool = nullptr;
  bool running = false;
  std::atomic<bool> should_stop;
  std::thread thr;
};

class global_thread_pool {
public:
  global_thread_pool() {
    std::lock_guard<std::mutex> lock(threads_mt);
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
      created_threads.emplace_back();
    }
  }

  void add_one_thread(thread_pool &thr_pool) {
    for (auto &worker : created_threads) {
      if (worker.try_to_join(thr_pool)) {
        return;
      }
    }

    created_threads.emplace_back(thr_pool);
  }

private:
  std::mutex threads_mt;
  std::list<worker_thread> created_threads;
};

global_thread_pool global_thr_pool;

void add_one_thread(thread_pool &thr_pool) {
  global_thr_pool.add_one_thread(thr_pool);
}
}
}
