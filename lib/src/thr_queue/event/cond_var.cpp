#include "../global_thr_pool_impl.h"
#include "lock_unlocker.h"
#include "thr_queue/coroutine.h"
#include "thr_queue/event/cond_var.h"

namespace game_engine {
namespace thr_queue {

static std::mutex cond_var_global_mt;
static std::deque<coroutine> cor_queue;

namespace event {
void condition_variable::wait(std::unique_lock<mutex> &lock) {
  assert(lock.owns_lock());
  std::unique_lock<std::mutex> lock_std(mt);
  global_thr_pool.yield([&] {
    // running_coroutine_or_yielded from still points to this coroutine even
    // after having yielded.
    lock_unlocker<std::unique_lock<mutex>> l_unlock_co_mt(lock);
    lock_unlocker<std::unique_lock<std::mutex>> l_unlock_std_mt(lock_std);

    waiting_cors.emplace_back(std::move(*running_coroutine_or_yielded_from));
  });
  lock.lock();
}

void condition_variable::notify() {
  std::unique_lock<std::mutex> mt_lock(mt);
  auto wc = std::move(waiting_cors);
  mt_lock.unlock();

  for (auto &cor : wc) {
    global_thr_pool.schedule(std::move(cor), true);
  }
}

condition_variable::~condition_variable() {
  std::lock_guard<std::mutex> lock(mt);
  assert(waiting_cors.size() == 0 &&
         "coroutines can't be destroyed by the condition variable.");
}
}
}
}
