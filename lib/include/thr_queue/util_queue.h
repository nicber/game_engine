#pragma once

#include "thr_queue/queue.h"

namespace game_engine {
namespace thr_queue {
/** \brief Returns a queue that automatically submits any work to the global
 * thread pool.
 */
queue& default_par_queue( );

/** \brief Returns a queue that automatically submits any work to the global
 * thread pool preserving the order in which the units of work are submitted.
 */
queue ser_queue( );
}
}
