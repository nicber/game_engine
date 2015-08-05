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
  if (ready()) {
    return;
  }
  auto &d = get_priv();
  boost::unique_lock<mutex> l(d.co_mt);
  d.cv.wait(l);
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