#pragma once

#include <atomic>
#include <boost/optional.hpp>
#include <memory>
#include <thr_queue/event/future.h>

namespace game_engine {
namespace aio {
struct aio_runtime_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct aio_buffer {
  size_t len;
  char *buf;
  bool own;

  aio_buffer(size_t len);
  aio_buffer(size_t len_, char *buf_);

  aio_buffer(aio_buffer &&other);
  aio_buffer &operator=(aio_buffer rhs);

  ~aio_buffer();

private:
  /** \brief Default construct for the state of a moved-from object.
   */
  aio_buffer();

  friend void swap(aio_buffer &lhs, aio_buffer &rhs);
};

using aio_buffer_ptr = std::shared_ptr<aio_buffer>;

template <typename T>
using aio_result_future = game_engine::thr_queue::event::future<T>;

template <typename T>
using aio_result_promise = game_engine::thr_queue::event::promise<T>;

class aio_operation_base {
public:
  virtual ~aio_operation_base();

  void set_perform_on_destruction(bool flag);
  bool get_perform_on_destruction() const;

protected:
  bool already_performed = false;
private:
  bool perform_on_destr = true;
};

template <typename T>
class aio_operation_t : public aio_operation_base
{
public:
  aio_result_future<T> perform();
  ~aio_operation_t();
protected:
  virtual aio_result_future<T> do_perform() = 0;
};
}
}

#include "aio.inl"