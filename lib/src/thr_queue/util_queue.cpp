#include "thr_queue/global_thr_pool.h"
#include "thr_queue/util_queue.h"

namespace game_engine {
namespace thr_queue {
static void sch_par_queue(queue &q) {
  queue qw(steal_work, q);
  schedule_queue(std::move(qw));
}

static void sch_ser_queue(queue &q) {
  queue qw(steal_work, q);
  schedule_queue(std::move(qw));
}

static queue def_par_queue(queue_type::parallel, sch_par_queue);

queue &default_par_queue() { return def_par_queue; }

queue ser_queue() {
  queue q(queue_type::serial, sch_ser_queue);
  return q;
}
}
}
