#pragma once

#include <utility>

namespace game_engine {
namespace thr_queue {
class functor {
public:
  virtual void operator()() = 0;
  virtual ~functor() {};
};

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
