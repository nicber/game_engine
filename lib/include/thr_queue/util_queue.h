#pragma once

#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
queue &default_par_queue();
queue ser_queue();
}
}
