#pragma once

#include "thr_queue/event/cond_var.h"
#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
void schedule_queue(queue q);
void schedule_queue_first(queue q);
void schedule_queue(queue q,
                    event::condition_variable &cv,
                    event::mutex &mt,
                    size_t min);
void schedule_queue_first(queue q,
                          event::condition_variable &cv,
                          event::mutex &mt,
                          size_t min);
}
}
