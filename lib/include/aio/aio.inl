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
lambda_aio_operation_t<F>::lambda_aio_operation_t(F func, bool may_block)
 :function(std::move(func))
 ,may_block_f(may_block)
{}

template<typename F>
lambda_aio_operation_t<F>::~lambda_aio_operation_t()
{
  this->perform_on_destruction_if_need();
}

template<typename F>
typename lambda_aio_operation_t<F>::aiorf
lambda_aio_operation_t<F>::do_perform()
{
  return function();
}

template<typename F>
bool
lambda_aio_operation_t<F>::may_block()
{
  return may_block_f;
}

template<typename F>
std::shared_ptr<lambda_aio_operation_t<F>>
make_aio_operation(F function, bool may_block)
{
  return std::make_unique<lambda_aio_operation_t<F>>(std::move(function), may_block);
}
}
}
