#pragma once

#include <deque>
#include <functional>
#include <mutex>

#include "thr_queue/future.h"

namespace game_engine {
namespace thr_queue {
class thread_pool;

enum class queue_type {
  serial,
  parallel
};

enum class block {
  yes,
  no
};

/** \brief A queue for organizing work.
 * There are two types of queues, parallel and serial.
 * Each works differently:
 * * A parallel queue is used for storing units of work with no chronological
 *   relation between themselves, and therefore can be run in parallel.
 *   It can be used for submiting batches of number crunching work, for
 *   example.
 * * A serial queue executes elements in a way such that the chronological
 *   relation between them is respected. That means that they are generally
 *   executed one after the other in one thread.
 */
class queue {
public:
  /** \brief Constructs a queue of a certain type. */
  queue(queue_type ty);

  queue &operator=(queue &&rhs);
  queue(queue &&rhs);

  ~queue() {};

  /** \brief Adds a function to be executed to the queue. */
  template <typename F> get_future_type<F> submit_work(F func);

  /** \brief Appends a queue to this one.
   * Depending on what type of queue is appended and what type this queue is
   * it's execution will be different.
   * A parallel queue appended to another parallel queue: the queues' work
   * will be merged.
   * A parallel queue appended to a serial queue: the queue will be added as
   * a whole work unit to this one. However, the appended queue's work will
   * be executed in parallel and the serial queue will wait for it to finish
   * completely.
   * A serial queue appended to a parallel queue: the serial queue will be
   * considered a whole unit of work to be executed in parallel with the
   * queue's work.
   * A serial queue appended to a serial queue: the queues will be merged.
   * */
  void submit_queue(queue q);

  /** \brief Run one unit of work from this queue.
   * Returns true if we did work.
   * */
  bool run_once();

  /** \brief Run everything from this queue in the caller's thread.
   * It's important to note that parallel queues will be run serially.
   * If that's not what you want, you run it there using the run_in_pool method.
   */
  void run_until_empty();

  /** \brief Starts a thread pool of a certain maximum size and runs this queue
   * there. Blocks or not according to the first parameter.
   */
  void run_in_pool(block bl, unsigned int max_pool_size);

  /** \brief Runs this queue in an already existing thread pool.
   * Blocks or not according to the first parameter.
   */
  void run_in_pool(block bl, thread_pool &thr_pool);

  queue_type type() const;

  struct base_work {
    virtual void operator()() = 0;
    virtual ~base_work() {};
  };

private:
  template <typename F> struct work : base_work {
    F func;
    get_promise_type<F> prom;

    void operator()() final override;

    work(F f, get_promise_type<F> p);
  };

  friend void run_queue_in_pool_impl(bool add_this_thread_if_block, block bl,
                                     queue q,
                                     std::unique_ptr<base_work> last_work,
                                     thread_pool &pool);
  friend void swap(queue &lhs, queue &rhs);
  std::recursive_mutex queue_mut;
  std::deque<std::unique_ptr<base_work> > work_queue;
  queue_type typ;
};

void swap(queue &lhs, queue& rhs);
}
}

#include "thr_queue/queue.inl"
