#include "global_thr_pool_impl.h"
#include <thr_queue/queue.h>
#include <vector>

namespace game_engine {
namespace thr_queue {

static std::vector<coroutine> queue_to_vec_cor(queue q) {
  std::vector<coroutine> cors;

  if (q.type() == queue_type::serial) {
    auto func = [q = std::move(q)]() mutable {
      q.run_until_empty();
    };
    cors.emplace_back(std::move(func));
  } else {
    cors.reserve(q.work_queue.size());
    for (auto &work : q.work_queue) {
      auto func = [work = std::move(work)]() mutable {
        (*work)();
      };
      cors.emplace_back(std::move(func));
    }
  }

  return cors;
}

void schedule_queue(queue q) {
  auto cors = queue_to_vec_cor(std::move(q));
  for (auto &cor : cors) {
    global_thr_pool.schedule(std::move(cor), false);
  }
}

void schedule_queue_first(queue q) {
  auto cors = queue_to_vec_cor(std::move(q));
  for (auto &cor : cors) {
    global_thr_pool.schedule(std::move(cor), true);
  }
}
}
}
