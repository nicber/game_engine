#pragma once

#include "thr_queue/queue.h"

namespace {

template <typename F> bool valid_function(const F &) { return true; }

template <typename T> bool valid_function(const std::function<T()> &f) {
  return bool(f);
}
}

namespace game_engine {
namespace thr_queue {
template <typename F> get_future_type<F> queue::submit_work(F func) {
  if (valid_function(func)) {
    throw std::runtime_error("invalid function passed");
  }

  get_promise_type<F> prom;
  auto future = prom.get_future();

  std::lock_guard<std::mutex> guard(queue_mut);
  work_queue.emplace_back(new work<typename std::result_of<F()>::type>(
      std::move(func), std::move(prom)));

  return future;
}

template <> void queue::work<void>::operator()() {
  try {
    func();
    prom.set_value();
  }
  catch (...) {
    prom.set_exception(std::current_exception());
  }
}

template <typename F> void queue::work<F>::operator()() {
  try {
    prom.set_value(func());
  }
  catch (...) {
    prom.set_exception(std::current_exception());
  }
}

template <typename T>
queue::work<T>::work(std::function<T()> f, std::promise<T> p)
    : func(std::move(f)), prom(std::move(p)) {}
}
}
