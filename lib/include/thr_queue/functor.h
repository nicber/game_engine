#pragma once

#include <memory>
#include <utility>

namespace game_engine {
namespace thr_queue {
/** \brief A class similar to std::function that isn't copyable by default.
 * This allows us to capture std::unique_ptrs or other move-only classes.
 */
class functor {
public:
  virtual void operator()() = 0;
  virtual ~functor() {};
};

/** \brief A template similar to std::function that is only movable by default.
 */
template <typename T>
class spec_functor : public functor {
public:
  spec_functor(T func) : function(std::move(func)) {}
  void operator()() final override { function(); }

private:
  T function;
};

using functor_ptr = std::unique_ptr<functor>;

template <typename F>
functor_ptr make_functor(F f)
{
  auto func = std::make_unique<spec_functor<F>>(std::move(f));
  return func;
}
}
}
