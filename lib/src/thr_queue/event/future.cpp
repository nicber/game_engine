#include <thr_queue/util_queue.h>
#include "../global_thr_pool_impl.h"
#include <thr_queue/event/future.h>

namespace game_engine {
namespace thr_queue {
namespace event {
void
future_promise_priv_shared::notify_all_cvs(functor_ptr func)
{
  scheduled_set_func = true;
  auto notify_work = [d = shared_from_this(), func = std::move(func)] {
    boost::unique_lock<mutex> l(d->mt);
    assert(d->prom_status == promise_status::alive_not_set);
    if (func != nullptr) {
      (*func)();
    }
    d->prom_status = promise_status::alive_set;
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
    default_par_queue().submit_work(std::move(notify_work));
  } else {
    notify_work();
  }
}

void promise<void>::set_value()
{
  if (this->d->prom_status == promise_status::alive_set) {
    throw promise_already_set("attempting to set an already set promise<void>");
  }

  this->d->notify_all_cvs(nullptr);
}

void future_generic_base::wait() const
{
  if (running_coroutine_or_yielded_from) {
    auto &d = get_priv();
    boost::unique_lock<mutex> l(d.mt);
    while (d.prom_status == promise_status::alive_not_set) {
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

std::exception_ptr
future_generic_base::get_exception() const
{
  if (running_coroutine_or_yielded_from) {
    auto &d = get_priv();
    boost::unique_lock<mutex> l(d.mt);
    while (d.prom_status == promise_status::alive_not_set) {
      d.cv.wait(l);
    }
    return d.except_ptr;
  } else {
    boost::promise<std::exception_ptr> prom;
    auto fut = prom.get_future();
    queue q(queue_type::parallel);
    q.submit_work([this, &prom] {
      auto ptr = get_exception();
      prom.set_value(std::move(ptr));
    });
    schedule_queue_first(std::move(q));
    return fut.get();
  }
}

bool future_generic_base::ready() const
{
  auto &d = get_priv();
  promise_status promst = d.prom_status;
  return promst == promise_status::alive_set
      || promst == promise_status::dead_set;
}
}
}
}
