#include "thr_queue/coroutine.h"
#include <condition_variable>
#include <deque>
#include <mutex>

namespace game_engine {
namespace thr_queue {
namespace event {
class condition_variable {
public:
  void notify();
  void wait();

private:
  std::mutex mt;
  std::deque<coroutine> waiting_cors;
};
}
}
}
