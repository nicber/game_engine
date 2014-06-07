#pragma once

#include "global_thr_pool.h"
#include "thr_queue/wait_event.h"

namespace game_engine {
namespace thr_queue {
template <typename E>
bool join_event(E& lhs, const E& rhs, unsigned int between) {
	return false;
}

template <typename E>
class event_handler {
void add_one(E ev);
bool try_run_one();
};

template <typename E>
void start_wait_for_event(E ev) {
	global_thr_pool.wait_event(std::move(ev));
}
}
}
