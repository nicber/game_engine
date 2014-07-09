#pragma once

#include "global_thr_pool_impl.h"

namespace game_engine {
namespace thr_queue {
template <typename F>
void global_thread_pool::yield(F func) {
  *after_yield = std::function<void()>(std::move(func));
  yield();
}
}
}
