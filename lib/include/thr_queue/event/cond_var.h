#pragma once

#include <deque>
#include "thr_queue/coroutine.h"
#include "thr_queue/event/mutex.h"
#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
namespace event {
class condition_variable {
public:
  void notify();
  void wait(boost::unique_lock<mutex> &lock);

  ~condition_variable();

private:
  boost::mutex mt;
  std::deque<coroutine> waiting_cors;
};
}
}
}
