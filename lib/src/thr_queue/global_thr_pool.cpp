#include <cassert>
#include "global_thr_pool_impl.h"
#include "thr_queue/event/cond_var.h"
#include <thr_queue/queue.h>
#include <vector>

namespace game_engine {
namespace thr_queue {
static std::vector<coroutine> queue_to_vec_cor(queue q,
                                               event::condition_variable *cv,
                                               event::mutex *mt,
                                               size_t min) {
  if (cv) {
    assert(mt);
  }

  std::vector<coroutine> cors;

  if (q.type() == queue_type::serial) {
    if (cv) {
      auto func = [ q = std::move(q), cv, mt, min ]() mutable {
	    for(size_t i = 0; q.run_once(); ++i) {
		    if (i >= min) {
			  boost::lock_guard<event::mutex> lock(*mt);
			  cv->notify();
			}
		}
      };
      cors.emplace_back(std::move(func));
    } else {
      auto func = [q = std::move(q)]() mutable {
        q.run_until_empty();
      };
      cors.emplace_back(std::move(func));
    }
  } else {
    cors.reserve(q.work_queue.size());
    if (cv) {
      auto atomic = std::make_shared<std::atomic<size_t>>(0);
      for (auto &work : q.work_queue) {
        auto func = [ work = std::move(work), atomic, cv, mt, min ]() mutable {
          (*work)();
          size_t atom_val = ++(*atomic);
          if (atom_val >= min) {
            boost::lock_guard<event::mutex> lock(*mt);
            cv->notify();
          }
        };
        cors.emplace_back(std::move(func));
      }
    } else {
      for (auto &work : q.work_queue) {
        auto func = [work = std::move(work)]() mutable {
          (*work)();
        };
        cors.emplace_back(std::move(func));
      }
    }
  }

  return cors;
}

static void do_schedule(queue &q,
                        event::condition_variable *cv,
                        event::mutex *mt,
                        size_t min,
                        bool first) {
  auto cors = queue_to_vec_cor(std::move(q), cv, mt, min);
  auto begin_move = std::make_move_iterator(cors.begin());
  auto end_move = std::make_move_iterator(cors.end());
  global_thr_pool.schedule(begin_move, end_move, first);
}

void schedule_queue(queue q) { do_schedule(q, nullptr, nullptr, 0, false); }

void schedule_queue_first(queue q) {
  do_schedule(q, nullptr, nullptr, 0, true);
}

void schedule_queue(queue q,
                    event::condition_variable &cv,
                    event::mutex &mt,
                    size_t min) {
  do_schedule(q, &cv, &mt, min, false);
}

void schedule_queue_first(queue q,
                          event::condition_variable &cv,
                          event::mutex &mt,
                          size_t min) {
  do_schedule(q, &cv, &mt, min, true);
}
}
}
