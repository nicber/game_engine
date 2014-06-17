#pragma once

#include <condition_variable>
#include <deque>
#include "thr_queue/coroutine.h"
#include "thr_queue/event/mutex.h"

namespace game_engine {
namespace thr_queue {
namespace event {
class condition_variable {
public:
  void notify();
  void wait(std::unique_lock<mutex> &lock);

  ~condition_variable();

private:
  std::mutex mt;
  std::deque<coroutine> waiting_cors;
};
}
}
}
