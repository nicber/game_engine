#pragma once

#include <type_traits>

namespace game_engine {
namespace thr_queue {
template <typename F>
coroutine::coroutine(F func)
{
  // If F is a subclass of queue::functor then we avoid wrapping it in a
  // spec_functor that will only call it. Instead, we just move it to the heap.
  using wrapped_functor =
      typename std::conditional<std::is_base_of<functor, F>::value,
                                F,
                                spec_functor<F>>::type;

  function =
      std::unique_ptr<wrapped_functor>(new wrapped_functor(std::move(func)));

  new (this) coroutine(std::move(function));
}
}
}
