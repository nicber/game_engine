#pragma once

#include <boost/optional.hpp>
#include "cond_var.h"
#include <list>
#include <memory>
#include <stdexcept>
#include <thr_queue/functor.h>

//shamelessly copied from boost::promise and boost::future

namespace game_engine {
namespace thr_queue {
namespace event {
struct future_promise_priv_shared {
  boost::mutex os_mt;
  mutex co_mt;
  condition_variable cv;
  std::list<std::weak_ptr<condition_variable>> when_any_callbacks;
  functor_ptr wait_callback = nullptr;
  std::exception_ptr except_ptr;
  bool promise_alive = true;
  bool set_flag = false;
};

template <typename R>
struct future_promise_priv : future_promise_priv_shared {
  boost::optional<R> val;
};

template <>
struct future_promise_priv<void> : future_promise_priv_shared {
};

struct promise_already_set : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct promise_dead_before_completion : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct future_was_emptied_before : std::runtime_error {
  using std::runtime_error::runtime_error;
};

template <typename R>
class future;

template <typename R>
class promise_base;

template <typename R>
void swap(promise_base<R> &lhs, promise_base<R> &rhs);

template <typename R>
class promise_base {
public:
  typedef R value_type;
  promise_base();
  promise_base &operator=(promise_base const& rhs) = delete;
  promise_base(promise_base const& rhs) = delete;
  ~promise_base();

  // Move support
  promise_base(promise_base &&rhs) noexcept;
  promise_base &operator=(promise_base &&rhs) noexcept;

  void set_exception(std::exception_ptr e);

  template <typename E>
  void set_exception(E e);

  template<typename F>
  void set_wait_callback(F f);

  future<R> get_future() const;

protected:
  friend void swap<R>(promise_base<R> &lhs, promise_base<R> &rhs);
  void notify_all_cvs();
  std::shared_ptr<future_promise_priv<R>> d;
};

template <typename R>
class promise : public promise_base<R> {
public:
  void set_value(R val);
};

template <>
class promise<void> : public promise_base<void> {
public:
  void set_value();
};

class future_generic_base {
protected:
  virtual future_promise_priv_shared &get_priv() const = 0;

public:
  void wait() const;

  bool ready() const;
};

template <typename R>
class future_base : public future_generic_base {
  future_promise_priv_shared &get_priv() const final override;
  
protected:
  future_base(std::shared_ptr<future_promise_priv<R>> d_);

  std::shared_ptr<future_promise_priv<R>> d;
};

template <typename R>
class future : public future_base<R> {
public:
  R get();

  const R &peek() const;

private:
  friend class promise_base<R>;
  friend class promise<R>;
  using future_base::future_base;
};

template <>
class future<void> : public future_base<void> {
  friend class promise_base<void>;
  friend class promise<void>;
  using future_base::future_base;
};

template <typename InIt>
typename std::allocator_traits<InIt>::reference_type wait_any(InIt first, InIt last);

template <typename InIt>
void wait_all(InIt first, InIt last);

template <typename... Rs>
size_t wait_any(const future<Rs>&...);

template <typename... Rs>
void wait_all(const future<Rs>&...);
}
}
}

#include "future.inl"