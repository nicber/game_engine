#pragma once

namespace game_engine {
namespace thr_queue {
class coroutine;

template <typename E> void start_wait_for_event(E ev);
}
}

#include "thr_queue/wait_event.inl"
