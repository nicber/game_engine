#include "thr_queue/queue.h"
#include "thr_queue/thread_pool.h"
#include "thr_queue/global_thr_pool.h"

namespace game_engine {
namespace thr_queue {
thread_local thread_pool *current_pool = nullptr;

thread_pool &get_thread_pool() { return *current_pool; }

void thread_pool::make_pool_bigger() {
  while (current_threads < max_pool_size) {
    add_one_thread(*this);
  }
}

thread_pool::thread_pool(unsigned int max_pool_siz)
    : max_pool_size(max_pool_siz) {}

void thread_pool::schedule_queue(queue q) {
  std::lock_guard<std::mutex> lock(mt);
  queues_to_run.emplace_back(std::move(q), 0);
  make_pool_bigger();
}

void thread_pool::schedule_queue_first(queue q) {
  std::lock_guard<std::mutex> lock(mt);
  queues_to_run.emplace_back(std::move(q), 0);
  make_pool_bigger();
}

void thread_pool::participate(kickable kick) {
  current_pool = this;
  {
    std::lock_guard<std::mutex> lock(mt);
    current_threads++;
    if (kick == kickable::no) {
      non_kickable_threads++;
    }
    should_exit[std::this_thread::get_id()] = false;
  }
  loop(kick);
  cv.notify_one();
}

void thread_pool::loop(kickable kick) {

  while (true) {
    std::unique_lock<std::mutex> lock(mt);

    while (queues_to_run.size() == 0) {
      if (kick == kickable::yes ||
          current_threads == non_kickable_threads) {
        current_threads--;
        should_exit.erase(std::this_thread::get_id());
        if (kick == kickable::no) {
          non_kickable_threads--;
        }
        return;
      } else {
        cv.wait(lock,
                [&]() { return current_threads == non_kickable_threads; });
      }
    }

    if (queues_to_run.front().first.type() == queue_type::serial) {
      auto q = std::move(queues_to_run.front().first);
      queues_to_run.pop_front();
      lock.unlock();

      q.run_until_empty();
    } else {
      queues_to_run.front().second++;
      auto &q = queues_to_run.front().first;

      lock.unlock();

      while (q.run_once())
        ;

      lock.lock();
      if (--queues_to_run.front().second == 0) {
        queues_to_run.pop_front();
      }
      lock.unlock();
    }
  }
}
}
}
