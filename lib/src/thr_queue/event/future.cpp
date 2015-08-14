#include <thr_queue/util_queue.h>
#include "../global_thr_pool_impl.h"
#include <thr_queue/event/future.h>

namespace game_engine {
namespace thr_queue {
namespace event {
void promise<void>::set_value()
{
  boost::lock_guard<boost::mutex> l(d->os_mt);
  if (d->set_flag) {
    std::ostringstream ss;
    ss << "attempting to set an already set promise<void>";
    LOG() << ss.str();
    throw promise_already_set(ss.str());
  }
  notify_all_cvs();
}

void future_generic_base::wait() const
{
  auto &d = get_priv();
  boost::unique_lock<mutex> l(d.co_mt);
  if (d.set_flag) {
    return;
  } else if (running_coroutine_or_yielded_from) {
    d.cv.wait(l);
  } else {
    boost::promise<void> prom;
    auto fut = prom.get_future();
    queue q(queue_type::parallel);
    q.submit_work([this, &prom] {
      wait();
      prom.set_value();
    });
    schedule_queue_first(std::move(q));
    fut.wait();
  }
}

bool future_generic_base::ready() const
{
  auto &d = get_priv();
  boost::lock_guard<boost::mutex> l(d.os_mt);
  return d.set_flag;
}
}
}
}