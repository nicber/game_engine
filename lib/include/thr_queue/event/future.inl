#pragma once

#include <algorithm>
#include <boost/iterator/indirect_iterator.hpp>
#include "future.h"
#include <logging/log.h>
#include <thr_queue/global_thr_pool.h>
#include <vector>

namespace game_engine {
namespace thr_queue {
namespace event {
template<typename R>
promise_base<R>::promise_base()
 :d(std::make_shared<future_promise_priv<R>>())
{}

template<typename R>
promise_base<R>::~promise_base()
{
  if (d) {
    boost::lock_guard<boost::mutex> l(d->os_mt);
    d->promise_alive = false;
    if (!d->set_flag && d.use_count() > 1) {
      const char *msg = "promise died before the future was set";
      LOG() << msg;
      d->except_ptr = std::make_exception_ptr(promise_dead_before_completion(msg));
    }
    notify_all_cvs();
  }
}

template<typename R>
promise_base<R>::promise_base(promise_base && rhs) noexcept
{
  promise_base<R> other;
  swap(*this, other);
  swap(*this, rhs);
}

template <typename R>
promise_base<R> &promise_base<R>::operator=(promise_base<R> &&rhs) noexcept
{
  swap(*this, rhs);
  return *this;
}

template<typename R>
void promise_base<R>::set_exception(std::exception_ptr e)
{
  boost::lock_guard<boost::mutex> l(d->os_mt);
  d->except_ptr = std::move(e);
  notify_all_cvs();
}

template<typename R>
template<typename E>
void promise_base<R>::set_exception(E e)
{
  set_exception(std::make_exception_ptr(std::move(e)));
}

template<typename R>
template<typename F>
void promise_base<R>::set_wait_callback(F f)
{
  boost::lock_guard<boost::mutex> l(d->os_mt);
  d->wait_callback = make_functor(std::move(f));
}

template<typename R>
future<R> promise_base<R>::get_future() const
{
  return future<R>(const_cast<promise_base<R>*>(this)->d);
}

template<typename R>
void game_engine::thr_queue::event::promise_base<R>::notify_all_cvs()
{
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
}

template<typename R>
void swap(promise_base<R>& lhs, promise_base<R>& rhs)
{
  swap(lhs.d, rhs.d);
}

template<typename InIt>
typename std::allocator_traits<InIt>::reference_type wait_any(InIt first, InIt last)
{
  return typename std::allocator_traits<InIt>::reference_type();
}

template<typename InIt>
void wait_all(InIt first, InIt last)
{
}

template<typename... Rs>
size_t wait_any(const future<Rs>&... futs)
{
  auto check_ready = [&]() -> boost::optional<size_t> {
    bool any_ready[] = {futs.ready()...};
    for (size_t i = 0; i < sizeof...(Rs); ++i) {
      if (any_ready[i]) {
        return i;
      }
    }
    return boost::none;
  };

  if (auto index = check_ready()) {
    return *index;
  }

  auto shared_cv = std::make_shared<condition_variable>();
  promise<void> cor_prom;
  future<void> cor_cut = cor_prom.get_future();

  std::vector<boost::unique_lock<boost::mutex>> locks;
  std::vector<std::vector<std::weak_ptr<condition_variable>>*> cv_vectors;
  locks.reserve(sizeof...(Rs));
  cv_vectors.reserve(sizeof...(Rs));

  auto preproc = [&](auto future) {
    auto &d = const_cast<std::remove_const_t<decltype(*future.d)>>(*future.d);
    locks.emplace_back(d->os_mt, boost::defer_lock);
    cv_vectors.emplace_back(&d->when_any_callbacks);
  };

  int l{(preproc(futs),0)...};
  boost::lock(begin(locks), end(locks));
  for (auto *cv_vec : cv_vectors) {
    cv_vec->emplace_back(shared_cv);
  }
}

template<typename... Rs>
void wait_all(const future<Rs>&... futs)
{
  struct waiter {
    waiter(const future_generic_base &fut_)
     :fut(fut_)
    {}

    ~waiter() {
      fut.wait();
    }
    const future_generic_base &fut;
  };
  waiter waiters[]{futs...};
}

template<typename R>
void promise<R>::set_value(R val)

{
  boost::lock_guard<boost::mutex> l(this->d->os_mt);
  if (this->d->set_flag) {
    throw promise_already_set("the promise has already been set to a value");
  }
  this->d->val = std::move(val);
  this->notify_all_cvs();
}

template<typename R>
future_promise_priv_shared & game_engine::thr_queue::event::future_base<R>::get_priv() const
{
  return const_cast<future_promise_priv<R>&>(*this->d);
}

template<typename R>
game_engine::thr_queue::event::future_base<R>::future_base(std::shared_ptr<future_promise_priv<R>> d_)
 :d(std::move(d_))
{}

template<typename R>
R future<R>::get()
{
  this->wait();
  boost::lock_guard<boost::mutex> l(this->d->os_mt);
  if (this->d->except_ptr) {
    std::rethrow_exception(this->d->except_ptr);
  } else if (this->d->val) {
    R val = std::move(*this->d->val);
    this->d->val = boost::none;
    return val;
  } else {
    throw future_was_emptied_before("the future was emptied before");
  }
}

template<typename R>
const R &future<R>::peek() const
{
  this->wait();
  boost::lock_guard<boost::mutex> l(this->d->os_mt);
  if (this->d->except_ptr) {
    std::rethrow_exception(this->d->except_ptr);
  } else if (this->d->val) {
    return *this->d->val;
  } else {
    throw future_was_emptied_before("the future was emptied before");
  }
}
}
}
}
