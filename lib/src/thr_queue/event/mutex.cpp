#include "../global_thr_pool_impl.h"
#include "lock_unlocker.h"
#include "thr_queue/event/mutex.h"

namespace game_engine {
namespace thr_queue {
namespace event {

void mutex::lock() {
  std::unique_lock<std::mutex> lock(mt, std::defer_lock);

  // we might have been scheduled by a thread that unlocked the lock, but that
  // doesn't guarantee that another thread didn't lock this lock before we had
  // time to run.
  while (true) {
    assert(!lock.owns_lock());
    lock.lock();

    if (!owning_cor) {
      owning_cor = running_coroutine_or_yielded_from;
      return;
    }

    global_thr_pool.yield([&] {
      lock_unlocker<std::mutex> l_unlock(lock);
      waiting_cors.emplace_back(std::move(*running_coroutine_or_yielded_from));
    });
    assert(!lock.owns_lock());
  }
}

bool mutex::try_lock() {
  std::unique_lock<std::mutex> lock(mt);

  if (owning_cor) {
    return false;
  } else {
    owning_cor = running_coroutine_or_yielded_from;
  }

  return true;
}

void mutex::unlock() {
  std::unique_lock<std::mutex> lock(mt);

  assert(running_coroutine_or_yielded_from == owning_cor);

  owning_cor = nullptr;

  if (waiting_cors.size()) {
    auto cor = std::move(waiting_cors.front());
    waiting_cors.pop_front();
    global_thr_pool.schedule(std::move(cor), true);
  }
}
}
}
}
