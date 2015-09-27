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
struct aio_runtime_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct aio_buffer : uv_buf_t {
  using size_type = decltype(uv_buf_t().len);
  /** \brief Default construct for the state of a moved-from, aka empty, object.
   */
  aio_buffer();

  aio_buffer(uv_buf_t &&buf);
  aio_buffer(size_type length);

  aio_buffer(const aio_buffer &) = delete;

  aio_buffer(aio_buffer &&other);
  aio_buffer &operator=(aio_buffer rhs);

  void append(aio_buffer other);

  uv_buf_t get_subbuffer(size_type shift);

  ~aio_buffer();

private:

  friend void swap(aio_buffer &lhs, aio_buffer &rhs);
};

using aio_buffer_ptr = std::shared_ptr<aio_buffer>;

struct perform_helper_base {
  void about_to_block();
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

template <typename T>
struct perform_helper : perform_helper_base {
  using type = T;
  using opt_fut_T = boost::optional<thr_queue::event::future<T>>;
  perform_helper(opt_fut_T &fut);

  void set_future(thr_queue::event::future<T> fut);

private:
  bool future_already_set() final override;
  opt_fut_T &fut_storage;
};

class aio_operation_base {
public:
  virtual ~aio_operation_base();

  void set_perform_on_destruction(bool flag);
  bool get_perform_on_destruction() const;

protected:
  virtual bool may_block() = 0;
  void replace_running_cor_and_jump(perform_helper_base &helper, thr_queue::coroutine work_cor);

  bool already_performed = false;
private:
  bool perform_on_destr = true;
};


template <typename T>
class aio_operation_t : public aio_operation_base
                       ,protected std::enable_shared_from_this<aio_operation_t<T>>
{
public:
  using aio_result_future = game_engine::thr_queue::event::future<T>;
  using aio_result_promise = game_engine::thr_queue::event::promise<T>;

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
 * an aio_operation. The lambda must comply with this signature:
 * event::future<T> function()
 * where T will become the type the resulting aio_operation returns.
 */
template <typename F>
aio_operation<lambda_type_t<F>> make_aio_operation(F function);
}
}

#include "aio.inl"
