#pragma once

#include <type_traits>

namespace game_engine {
namespace thr_queue {
template <typename F>
coroutine::coroutine(F func, coroutine_type cor_typ)
    : coroutine() {
  typ = cor_typ;

  using wrapped_functor =
      typename std::conditional<std::is_base_of<functor, F>::value,
                                F,
                                spec_functor<F>>::type;

  function =
      std::unique_ptr<wrapped_functor>(new wrapped_functor(std::move(func)));

  stack = std::unique_ptr<char[]>(new char[default_stacksize()]);

  ctx = boost::context::make_fcontext(stack.get() + default_stacksize() - 1,
                                      default_stacksize(),
                                      [](intptr_t ptr) {
    auto *reint_ptr = reinterpret_cast<functor *>(ptr);
    (*reint_ptr)();
  });
}
}
}
