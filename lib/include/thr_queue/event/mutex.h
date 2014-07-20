#pragma once

#include <atomic>
#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
class coroutine;

namespace event {
class mutex {
public:
  ~mutex();
  void lock();
  bool try_lock();
  void unlock();

private:
  boost::mutex mt;
  coroutine *owning_cor = nullptr;
  std::deque<coroutine> waiting_cors;
};
}
}
}
