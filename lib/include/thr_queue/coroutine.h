#pragma once

#include "thr_queue/functor.h"

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

struct cor_data;
struct cor_data_deleter {
  void operator()(cor_data *);
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

  ~coroutine();

  /** \brief Construct a coroutine based on a functor. */
  coroutine(functor_ptr func);

  coroutine(coroutine &&other) = default;
  coroutine &operator=(coroutine &&rhs) = default;
  
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

private:
  /** \brief Constructs a master coroutine. */
  coroutine();

  friend class global_thread_pool;
  friend class generic_worker_thread;
  friend class platform::worker_thread_impl;
  friend struct cor_data;

private:
	std::unique_ptr<cor_data, cor_data_deleter> data_ptr;
};
}
}

#include "thr_queue/coroutine.inl"
