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

  /** \brief Construct a coroutine based on a functor. */
  coroutine(std::unique_ptr<functor> func);

  coroutine(coroutine &&other) noexcept;
  coroutine &operator=(coroutine &&rhs) noexcept;
  ~coroutine();
  
  /** \brief Returns the coroutine's type.
   */
  coroutine_type type() const;
  
  /** \brief Switches from this coroutine to the one passed as an argument.
   */
  void switch_to_from(coroutine &from);

  /** \brief Makes it so the coroutine cannot be run by the passed thread.
   * The forbidden thread is reset once the coroutine is run by any thread.
   * There can only be one forbidden thread per coroutine.
   */
  void set_forbidden_thread(worker_thread *thr);

  /** \brief Returns whether this coroutine can be run by the passed thread. */
  bool can_be_run_by_thread(worker_thread *thr) const;

  std::intptr_t get_id() const noexcept;

  friend void swap(coroutine &lhs, coroutine &rhs);

  struct stackctx;
private:
  /** \brief Constructs a master coroutine. */
  coroutine();

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

  //used in the linux implementation of blocking aio operations.
  //see aio_operation_t<T>::perform() for further information.
  worker_thread *forbidden_thread = nullptr;
};
}
}

#include "thr_queue/coroutine.inl"
