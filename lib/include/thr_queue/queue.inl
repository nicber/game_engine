#pragma once

#include "thr_queue/queue.h"
#include <type_traits>

namespace {
// used for knowing if a function is callable or not.
template <typename F>
bool valid_function(const F &) {
  return true;
}

template <typename T>
bool valid_function(const std::function<T()> &f) {
  return bool(f);
}
}

namespace game_engine {
namespace thr_queue {
template <typename F>
event::future<typename queue::work<F>::result_type> queue::submit_work(F func) {
  if (!valid_function(func)) {
    throw std::runtime_error("invalid function passed");
  }

  boost::lock_guard<boost::recursive_mutex> guard(queue_mut);

  auto work = std::unique_ptr<queue::work<F>>(
      new queue::work<F>(std::move(func),
                         typename event::promise<
                           typename queue::work<F>::result_type>()));
  auto fut = work->prom.get_future();
  work_queue.emplace_back(work.release());
  if (cb_added) {
    cb_added(*this);
  }

  return fut;
}

// workaround for the lack of partial function specialization.
// T is the type of the value that we have to store in the promise.
// F is the function.
template <typename F, typename T>
struct worker {
  static void do_work_and_store(F &f, event::promise<F> &prom) {
    prom.set_value(f());
  }
};
template <typename F>
struct worker<F, void> {
  static void do_work_and_store(F &f, event::promise<void> &prom) {
    f();
    prom.set_value();
  }
};

template <typename F>
void queue::work<F>::operator()() {
  try {
    worker<F, result_type>::do_work_and_store(func, prom);
  }
  catch (std::exception &e) {
    LOG() << "queue worker caught exception: " << e.what();
    prom.set_exception(std::current_exception());
    abort();
  }
}

template <typename F>
queue::work<F>::work(F f, event::promise<result_type> p)
    : prom(std::move(p)), func(std::move(f)) {}
}
}
