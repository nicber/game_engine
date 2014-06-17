#pragma once

#include <mutex>

namespace game_engine {
namespace thr_queue {
namespace event {

template <typename L>
class lock_unlocker {
public:
  lock_unlocker(std::unique_lock<L> &l) : lock(l) {}

  ~lock_unlocker() {
    assert(lock.owns_lock());
    lock.unlock();
  }

private:
  std::unique_lock<L> &lock;
};
}
}
}
