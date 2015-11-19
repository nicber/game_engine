#pragma once

#include "thr_queue/functor.h"

#include <boost/context/all.hpp>
#include <chrono>
#include <memory>

namespace game_engine {
namespace thr_queue {
namespace platform {
class worker_thread_impl;
}
class worker_thread;

enum class coroutine_type {
  user,
  master
};

/** \brief A class that represents a coroutine.
 * No data transfer is implemented by this class. That is the responsibility
 * of its user.
 */
class coroutine {
public:
  /** \brief Constructs a coroutine that executes func when switched to. Its type
   * will be cor_typ.
   */
  template <typename F>
  coroutine(F func);

  coroutine(coroutine &&other) noexcept;
  coroutine &operator=(coroutine &&rhs) noexcept;
  ~coroutine();
  
  /** \brief Returns the coroutine's type.
   */
  coroutine_type type() const;
  
  /** \brief Switches from this coroutine to the one passed as an argument.
   */
  void switch_to_from(coroutine &from);

  std::intptr_t get_id() const noexcept;

  friend void swap(coroutine &lhs, coroutine &rhs);

  struct stackctx;
private:
  /** \brief Constructs a master coroutine. */
  coroutine();

  /** \brief Helper for the templated constructor. */
  coroutine(std::unique_ptr<functor> func);

  friend class global_thread_pool;
  friend class generic_worker_thread;
  friend class platform::worker_thread_impl;

  static void finish_coroutine();

private:
  union {
    boost::context::fcontext_t ctx;
    std::unique_ptr<stackctx> stack_and_ctx;
  };
  std::unique_ptr<functor> function;

  coroutine_type typ;
  worker_thread *bound_thread = nullptr;
};
}
}

#include "thr_queue/coroutine.inl"
