#include <atomic>
#include <mutex>

namespace game_engine {
namespace thr_queue {
class coroutine;

namespace event {
class mutex {
public:
  void lock();
  bool try_lock();
  void unlock();

private:
  std::mutex mt;
  coroutine *owning_cor = nullptr;
  std::deque<coroutine> waiting_cors;
};
}
}
}
