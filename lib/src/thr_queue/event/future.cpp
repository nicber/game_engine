#include <thr_queue/util_queue.h>
#include "../global_thr_pool_impl.h"
#include <thr_queue/event/future.h>

namespace game_engine {
namespace thr_queue {
namespace event {
void
future_promise_priv_shared::notify_all_cvs(functor_ptr func)
{
  auto notify_work = [d = shared_from_this(), func = std::move(func)] {
    boost::unique_lock<mutex> l(d->mt);
    (*func)();
    d->set_flag = true;
    d->cv.notify();
    for (auto it = d->when_any_callbacks.begin(); it != d->when_any_callbacks.end();) {
      std::shared_ptr<condition_variable> cv_ptr(*it);
      if (!cv_ptr) {
        it = d->when_any_callbacks.erase(it);
      } else {
        cv_ptr->notify();
        ++it;
      }
    }
  };

  if (!running_coroutine_or_yielded_from) {
    default_par_queue().submit_work(std::move(notify_work)).wait();
  } else {
    notify_work();
  }
}

void promise<void>::set_value()
{
  std::exception_ptr except_ptr = nullptr;
  this->d->notify_all_cvs(make_functor([&except_ptr, d = this->d] {
    if (d->set_flag) {
       except_ptr = std::make_exception_ptr(
                    promise_already_set("attempting to set an already set promise<void>"));
       return;
    }
  }));
  if (except_ptr) {
    std::rethrow_exception(except_ptr);
  }
}

void future_generic_base::wait() const
{
  if (running_coroutine_or_yielded_from) {
    auto &d = get_priv();
    boost::unique_lock<mutex> l(d.mt);
    if (d.set_flag) {
      return;
    } else {
      d.cv.wait(l);
    }
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
  boost::lock_guard<mutex> l(d.mt);
  return d.set_flag;
}
}
}
}
