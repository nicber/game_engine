#include <cassert>
#include <iostream>
#include "thr_queue/global_thr_pool.h"
#include "thr_queue/util_queue.h"

namespace game_engine {
namespace thr_queue {
static void sch_par_queue(queue &q) {
  queue qw(steal_work, q);
  schedule_queue(std::move(qw));
}

static queue def_par_queue(queue_type::parallel, sch_par_queue);
queue &default_par_queue() { return def_par_queue; }

struct ser_queue_comm {
  event::mutex mt_exec;
  std::mutex mt_qu;
  queue qu = queue(queue_type::serial);
  bool cor_sched = false;
};

queue ser_queue() {
  auto comm = std::make_shared<ser_queue_comm>();

  queue q(queue_type::serial, [comm = std::move(comm)](queue & q) {
    std::lock_guard<std::mutex> lock(comm->mt_qu);

    comm->qu.append_queue({steal_work, q});

    if (comm->cor_sched) {
      return;
    }

    queue q_ser(queue_type::serial);

    q_ser.submit_work([comm]() mutable {
      std::lock_guard<event::mutex> lock_exec(comm->mt_exec);
      std::unique_lock<std::mutex> lock(comm->mt_qu);
      comm->cor_sched = false;
      auto q_work = std::move(comm->qu);
      comm->qu = queue(queue_type::serial);
      lock.unlock();

      q_work.run_until_empty();
    });

    schedule_queue(std::move(q_ser));
    comm->cor_sched = true;
  });

  return q;
}
}
}
