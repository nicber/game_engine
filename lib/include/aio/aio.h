#pragma once

#include <atomic>
#include <boost/optional.hpp>
#include <memory>
#include <thr_queue/event/future.h>
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

class aio_operation_base {
public:
  virtual ~aio_operation_base();

  void set_perform_on_destruction(bool flag);
  bool get_perform_on_destruction() const;

protected:
  virtual bool may_block() = 0;
  bool already_performed = false;
private:
  bool perform_on_destr = true;
};

template <typename T>
class aio_operation_t : public aio_operation_base
{
public:
  using aio_result_future = game_engine::thr_queue::event::future<T>;
  using aio_result_promise = game_engine::thr_queue::event::promise<T>;

  aio_result_future perform();
  virtual ~aio_operation_t() = 0;

protected:
  virtual aio_result_future do_perform() = 0;
  void perform_on_destruction_if_need();
};

template <typename T>
using aio_operation = std::unique_ptr<aio_operation_t<T>>;

/** \brief Implementation detail for make_aio_operation. Do not use.
 */
template <typename F>
class lambda_aio_operation_t : public aio_operation_t<typename std::result_of_t<F()>::value_type>
{
  using aiorf = typename aio_operation_t<typename std::result_of_t<F()>::value_type>::aio_result_future;
public:
  lambda_aio_operation_t(F func, bool may_block);
  ~lambda_aio_operation_t() final override;

protected:
  aiorf do_perform() final override;
  bool may_block() final override;

private:
  F function;
  bool may_block_f;
};

/** \brief This function takes a lambda compatible and wraps it into
 * an aio_operation. The lambda must comply with this signature:
 * event::future<T> function()
 * where T will become the type the resulting aio_operation returns.
 */
template <typename F>
std::unique_ptr<lambda_aio_operation_t<F>> make_aio_operation(F function, bool may_block);
}
}

#include "aio.inl"
