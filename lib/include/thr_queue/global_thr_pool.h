#pragma once

#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
/** \brief A function that schedules the queue q is the global thread pool and forgets
 * about it. The queue is responsible for informing, if necessary, about it's
 * completion.
 */
void schedule_queue( queue q );

/** \brief A function that schedules the queue q is the global thread pool and forgets
 * about it. The queue is responsible for informing, if necessary, about it's
 * completion. The queue is scheduled with maximum priority.
 */
void schedule_queue_first( queue q );

/** \brief A function that schedules a queue in the global thread pool and then
 * notifies using the condition variable passed as an argument when at least
 * min units of work have been completed.
 */
void schedule_queue( queue q, event::condition_variable& cv, event::mutex& mt, size_t min );

/** \brief A function that schedules a queue in the global thread pool and then
 * notifies using the condition variable passed as an argument when at least
 * min units of work have been completed.  The queue is scheduled with maximum
 * priority.
 */
void schedule_queue_first( queue q, event::condition_variable& cv, event::mutex& mt, size_t min );
}
}
