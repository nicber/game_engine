#pragma once

#include "thr_queue/functor.h"

#include <boost/context/all.hpp>
#include <chrono>
#include <memory>

namespace game_engine {
namespace thr_queue {

enum class coroutine_type {
  io,
  cpu,
  master
};

class coroutine {
public:
  template <typename F>
  coroutine(F func, coroutine_type cor_typ = coroutine_type::io);

  coroutine(coroutine &&other);
  coroutine &operator=(coroutine &&rhs);

  ~coroutine();

  std::chrono::high_resolution_clock::time_point creation_time() const;

  coroutine_type type() const;

  void switch_to();

  void make_current_coroutine();

  friend void swap(coroutine &lhs, coroutine &rhs);

private:
  coroutine(coroutine_type cor_typ);
  friend class worker_thread;

private:
  static size_t default_stacksize();

  boost::context::fcontext_t *ctx;
  std::unique_ptr<char[]> stack;
  std::unique_ptr<functor> function;

  coroutine_type typ;
  std::chrono::high_resolution_clock::time_point creation_tim;
};

void set_cor_type(coroutine_type cor_typ);
}
}

#include "thr_queue/coroutine.inl"
