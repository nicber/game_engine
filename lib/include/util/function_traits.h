#pragma once
#include <tuple>

namespace game_engine {
namespace util {
// gotten from:
// http://meh.schizofreni.co/programming/magic/2013/01/23/function-pointer-from-lambda.html

template <typename Function>
struct function_traits
  : public function_traits<decltype(&Function::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)> {
  using function_type = ReturnType(Args...);
  using return_type = ReturnType;
  using args = std::tuple<Args...>;
  using number_args = std::integral_constant<size_t, sizeof...(Args)>;
};
}
}