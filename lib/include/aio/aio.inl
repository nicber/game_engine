#pragma once

#include "aio.h"

namespace game_engine {
namespace aio {
template<typename T>
typename aio_operation_t<T>::aio_result_future
aio_operation_t<T>::perform()
{
  assert(!already_performed);
  already_performed = true;
  return do_perform();
}

template<typename T>
aio_operation_t<T>::~aio_operation_t()
{
}

template<typename T>
void 
aio_operation_t<T>::perform_on_destruction_if_need()
{
  if (get_perform_on_destruction() && !already_performed) {
    perform();
  }
}

template<typename F>
lambda_aio_operation_t<F>::lambda_aio_operation_t(F func)
 :function(std::move(func))
{}

template<typename F>
game_engine::aio::lambda_aio_operation_t<F>::~lambda_aio_operation_t()
{
  perform_on_destruction_if_need();
}

template<typename F>
typename lambda_aio_operation_t<F>::aio_result_future
game_engine::aio::lambda_aio_operation_t<F>::do_perform()
{
  return function();
}

template<typename F>
std::unique_ptr<lambda_aio_operation_t<F>>
game_engine::aio::make_aio_operation(F function)
{
  return std::make_unique<lambda_aio_operation_t<F>>(std::move(function));
}
}
}
