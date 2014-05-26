#pragma once

#include "thr_queue/queue.h"
#include <type_traits>

namespace {
// used for knowing if a function is callable or not.
template <typename F> bool valid_function(const F &) { return true; }

template <typename T> bool valid_function(const std::function<T()> &f) {
  return bool(f);
}
}

namespace game_engine {
namespace thr_queue {
template <typename F> get_future_type<F> queue::submit_work(F func) {
  if (!valid_function(func)) {
    throw std::runtime_error("invalid function passed");
  }

  get_promise_type<F> prom;
  auto future = prom.get_future();

  std::lock_guard<std::recursive_mutex> guard(queue_mut);
  work_queue.emplace_back(new work<F>(std::move(func), std::move(prom)));

  return future;
}

// workaround for the lack of partial function specialization.
// T is the type of the value that we have to store in the promise.
// F is the function.
template <typename F, typename T> struct worker {
  static void do_work_and_store(F &f, get_promise_type<F> &prom) {
    prom.set_value(f());
  }
};
template <typename F> struct worker<F, void> {
  static void do_work_and_store(F &f, get_promise_type<F> &prom) {
    f();
    prom.set_value();
  }
};

template <typename F> void queue::work<F>::operator()() {
  try {
    worker<F, typename std::result_of<F()>::type>::do_work_and_store(func,
                                                                     prom);
  }
  catch (...) {
    prom.set_exception(std::current_exception());
  }
}

template <typename F>
queue::work<F>::work(F f, get_promise_type<F> p)
    : func(std::move(f)), prom(std::move(p)) {}
}
}
