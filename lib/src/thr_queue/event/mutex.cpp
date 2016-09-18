#include "../global_thr_pool_impl.h"
#include "better_lock.h"
#include "lock_unlocker.h"
#include "thr_queue/event/mutex.h"

namespace game_engine {
namespace thr_queue {
namespace event {

mutex::~mutex() {
  boost::lock_guard<boost::mutex> lock(mt);
  assert(waiting_cors.size() == 0 &&
         "coroutines would get stuck trying to lock a non-existing mutex");
  assert(owning_cor == nullptr && "can't destroy a locked mutex");
}

void mutex::lock() {
  better_lock lock(mt, boost::defer_lock);

  // we might have been scheduled by a thread that unlocked the lock, but that
  // doesn't guarantee that another thread didn't lock this lock before we had
  // time to run.
  while (true) {
    assert(!lock.owns_lock());
    assert(running_coroutine);
    lock.lock();

    if (!owning_cor) {
      owning_cor = running_coroutine;
      break;
    }

    global_thr_pool.yield([&](coroutine running) {
      lock_unlocker<better_lock> l_unlock(lock);
      waiting_cors.emplace_back(std::move(running));
    });
  }

  assert(lock.owns_lock());
}

bool mutex::try_lock() {
  boost::unique_lock<boost::mutex> lock(mt);

  if (owning_cor) {
    return false;
  } else {
    owning_cor = running_coroutine;
  }

  return true;
}

void mutex::unlock() {
  boost::unique_lock<boost::mutex> lock(mt);

  assert(running_coroutine == owning_cor);

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
