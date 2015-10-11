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
  coroutine(F func, coroutine_type cor_typ = coroutine_type::user);

  coroutine(coroutine &&other) noexcept;
  coroutine &operator=(coroutine &&rhs) noexcept;
  ~coroutine();
  
  /** \brief Returns the coroutine's type.
   */
  coroutine_type type() const;
  
  /** \brief Switches from this coroutine to the one passed as an argument.
   */
  void switch_to_from(coroutine &from);
  
  friend void swap(coroutine &lhs, coroutine &rhs);

private:
  /** \brief Constructs a master coroutine. */
  coroutine();
  friend class generic_worker_thread;
  friend class platform::worker_thread_impl;

  /** \brief Changes the coroutine's type to that passed as an argument.
   * It to the master coroutine, changes the coroutine's type and calls
   * schedule for it to be scheduled with maximum priority.
   */
  friend void set_cor_type(coroutine_type cor_typ);
  
  static size_t default_stacksize();
  static void finish_coroutine();

private:
  /** \brief A pointer to the boost context that handles the context switching.
   * If this is a master coroutine then it's been new'ed and needs to be
   * deleted by the destructor. Otherwise it's stored in the topmost part of the stack
   * and just deleting that takes care of the context.
   */
  boost::context::fcontext_t ctx;
  std::unique_ptr<char[]> stack;
  std::unique_ptr<functor> function;

  coroutine_type typ;
};
void set_cor_type(coroutine_type cor_typ);
}
}

#include "thr_queue/coroutine.inl"
