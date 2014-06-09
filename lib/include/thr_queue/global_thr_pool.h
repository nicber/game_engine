#pragma once

#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
void schedule_queue(queue q);
void schedule_queue_first(queue q);
}
}
