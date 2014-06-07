#pragma once

#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
void schedule_queue(queue q);
void schedule_queue_first(queue q);

template <typename E>
void wait_event(E ev);

template <typename E>
class event_waiter {
public:
	void add_one(E ev);
	void wait();
};
}
}
