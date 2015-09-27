#pragma once

#include <atomic>
#include <boost/optional.hpp>
#include <memory>
#include <thr_queue/coroutine.h>
#include <thr_queue/event/future.h>
#include <util/function_traits.h>
#include <uv.h>

namespace game_engine {
namespace aio {
/** \brief A class that is used for all exceptions that are caused by IO failures.
 */
struct aio_runtime_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

/** \brief Represents a buffer that owns its contents.
 * It inherits from uv_buf_t and has been designed and array of aio_buffers can
 * be casted into an array of uv_buf_t without problems.
 */
struct aio_buffer : uv_buf_t {
  using size_type = decltype(uv_buf_t().len);
  /** \brief Default construct for the state of a moved-from, aka empty, object.
   */
  aio_buffer();

  /** \brief Constructs an aio_buffer from an uv_buf_t. It assumes that
   * the passed buffer owns the storage it points to. Use with caution.
   */
  explicit aio_buffer(uv_buf_t &&buf);

  /** \brief Constructs an uninitialized buffer of size length.
   */
  aio_buffer(size_type length);

  aio_buffer(const aio_buffer &) = delete;

  aio_buffer(aio_buffer &&other);
  aio_buffer &operator=(aio_buffer rhs);

  /** \brief Concatenates this buffer and the one passed as an argument and
   * stores the resulting buffer in this one.
   * It allocates memory if it is necessary.
   */
  void append(aio_buffer other);

  /** \brief Utility function that is useful for interfacing with libuv.
   * Often libuv calls a user callback that is meant to allocate a buffer,
   * this function creates an uv_buf_t that points to a subbuffer that is 
   * shifted to the right, that is, it starts 'shift' bytes after the start
   * of this buffer and its length 'shift' bytes shorter.
   */
  uv_buf_t get_subbuffer(size_type shift);

  ~aio_buffer();

private:

  friend void swap(aio_buffer &lhs, aio_buffer &rhs);
};

using aio_buffer_ptr = std::shared_ptr<aio_buffer>;

struct perform_helper_base {
  /** \brief In unix it immediately schedules the coroutine that called the aio_operation
   * in the global thread pool.
   * In Windows, it schedules an APC that will run when the thread is blocked for IO.
   * That APC is responsible for scheduling the coroutine in the global thread pool.
   * This lets us only schedule the coroutine to another thread only if the operation
   * actually blocks.
   */
  void about_to_block();

  /** \brief In Windows it is necessary to call this function to cleanup the APC
   * that was scheduled by about_to_block().
   */
  void cant_block_anymore();

  perform_helper_base() = default;
  perform_helper_base(const perform_helper_base &) = delete;
  perform_helper_base &operator=(const perform_helper_base &) = delete;

  ~perform_helper_base();
protected:
  virtual bool future_already_set() = 0;
private:
  boost::optional<thr_queue::coroutine> caller_coroutine;
  friend class aio_operation_base;
};

/** \brief Class that abstracts what is done to blocking aio_operations
 * to make them appear async to coroutines.
 * In more details: The aio_operation is supposed to call about_to_block()
 * when it is about to call an IO operation that can block. After it does
 * that the coroutine that called the aio_operation will be scheduled to run
 * in another thread. This gives it the illusion that it is not blocking.
 * After the blocking IO function returns cant_block_anymore() should be called
 * to perform any necessary cleanup.
 */
template <typename T>
struct perform_helper : perform_helper_base {
  using type = T;
  using opt_fut_T = boost::optional<thr_queue::event::future<T>>;
  perform_helper(opt_fut_T &fut);

  /** \brief Sets the future that is to be returned by aio_operation::perform().
   */
  void set_future(thr_queue::event::future<T> fut);

private:
  bool future_already_set() final override;
  opt_fut_T &fut_storage;
};

class aio_operation_base {
public:
  virtual ~aio_operation_base();

  /** \brief Sets the flag of whether the operation should be performed
   * by the destructor if it has not been performed yet.
   */
  void set_perform_on_destruction(bool flag);

  /** \brief Returns whether the operation should be performed by the destructor
   * if it has not been performed yet.
   */
  bool get_perform_on_destruction() const;

protected:
  virtual bool may_block() = 0;
  void replace_running_cor_and_jump(perform_helper_base &helper, thr_queue::coroutine work_cor);

  bool already_performed = false;
private:
  bool perform_on_destr = true;
};

/** \brief A class that represents an aio operation that returns a T.
 */
template <typename T>
class aio_operation_t : public aio_operation_base
                       ,protected std::enable_shared_from_this<aio_operation_t<T>>
{
public:
  using aio_result_future = game_engine::thr_queue::event::future<T>;
  using aio_result_promise = game_engine::thr_queue::event::promise<T>;

  /** \brief Runs the operation itself and returns a future with the result.
   */
  aio_result_future perform();

  virtual ~aio_operation_t() = 0;

protected:
  virtual aio_result_future do_perform_nonblock() = 0;
  virtual void do_perform_may_block(perform_helper<T> &helper) = 0;
  void perform_on_destruction_if_need();

};

template <typename T>
using aio_operation = std::shared_ptr<aio_operation_t<T>>;

template <typename F, bool>
struct lambda_type {
  using type = typename util::function_traits<F>::return_type::value_type;
};

template <typename F>
struct lambda_type<F, false> {
  using type = std::tuple_element_t<0, typename util::function_traits<F>::args>;
};

template <typename F>
using lambda_type_cond_help = typename std::is_same<typename util::function_traits<F>::number_args, std::integral_constant<size_t,0>>;

template <typename F>
using lambda_type_t = typename lambda_type<F, lambda_type_cond_help<F>::value>::type;

/** \brief This function takes a lambda compatible and wraps it into
 * an aio_operation. The lambda must comply with one of these signatures:
 * - event::future<T> function()
 * - void function(perform_helper<T> &)
 * where T will become the type the resulting aio_operation returns.
 */
template <typename F>
aio_operation<lambda_type_t<F>> make_aio_operation(F function);
}
}

#include "aio.inl"
