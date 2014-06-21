#pragma once

#include <mutex>

namespace game_engine {
namespace thr_queue {
namespace event {
class better_lock {
public:
  better_lock(std::mutex &m, std::defer_lock_t) : mt(m) {}

  ~better_lock() {
    if (already_locking) {
      unlock();
    }
  }

  void lock() {
    assert(!already_locking);
    mt.lock();
    already_locking = true;
  }

  void unlock() {
    // ensures that every thread that sees that the mutex is unlocked will also
    // see already_locking = false. This is not guaranteed by unique_lock.
    assert(already_locking);
    already_locking = false;
    mt.unlock();
  }

  bool owns_lock() { return already_locking; }

private:
  std::mutex &mt;
  bool already_locking = false;
};
}
}
}
