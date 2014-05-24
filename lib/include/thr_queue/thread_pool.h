#pragma once

#include "thr_queue/queue.h"
#include <thread>

namespace game_engine {
namespace thr_queue {
class thread_pool {
public:
  thread_pool(unsigned int max_pool_size);
  void schedule_queue(queue q);

  void participate();
  void remove_thread(std::thread::id thr);
};

thread_pool& get_thread_pool();
}
}
