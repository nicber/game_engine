#pragma once

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
}
}
