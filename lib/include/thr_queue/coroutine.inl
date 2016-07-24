#pragma once

#include "coroutine.h"
#include "functor.h"
#include <type_traits>

namespace game_engine {
namespace thr_queue {
template <typename F>
functor_ptr
wrap_func(F func)
{
  // If F is a subclass of queue::functor then we avoid wrapping it in a
  // spec_functor that will only call it. Instead, we just move it to the heap.
  using wrapped_functor =
    typename std::conditional<std::is_base_of<functor, F>::value,
    F,
    spec_functor<F>>::type;

  auto wf_tmp = std::make_unique<wrapped_functor>(std::move(func));
  return functor_ptr(wf_tmp.release());
}

template <typename F>
coroutine::coroutine(F func)
 :coroutine(wrap_func(std::move(func)))
{}
}
}
