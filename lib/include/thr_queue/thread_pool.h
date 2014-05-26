#pragma once

#include "thr_queue/queue.h"
#include <deque>
#include <thread>
#include <unordered_map>

namespace game_engine {
namespace thr_queue {
enum class kickable {
  yes,
  no
};

class thread_pool {
public:
  thread_pool(unsigned int max_pool_siz);
  void schedule_queue(queue q);
  void schedule_queue_first(queue q);

  void participate(kickable kick);
  void remove_thread(std::thread::id thr);

  void loop(kickable kick);

private:
  /** \brief Increases the pool size if necessary.
   * Assumes the lock has already been acquired.
   * */
  void make_pool_bigger();

  std::condition_variable cv;
  std::mutex mt;
  std::deque<std::pair<queue, unsigned int> > queues_to_run;
  std::unordered_map<std::thread::id, bool> should_exit;
  const unsigned int max_pool_size;
  unsigned int current_threads = 0;
  unsigned int non_kickable_threads = 0;
};

thread_pool &get_thread_pool();
}
}
