#pragma once

#include <cstddef>
#include <signal.h>

namespace game_engine {
namespace thr_queue {
namespace platform {
struct allocated_stack {
  void plat_release();
};
};
}
}
