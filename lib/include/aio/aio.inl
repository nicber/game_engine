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
  if (!may_block()) {
    return do_perform_nonblock();
  } else {
    boost::optional<aio_result_future> fut_storage;
    auto helper = std::make_unique<perform_helper<T>>(fut_storage);
    auto helper_ptr = helper.get();
    thr_queue::coroutine work_cor([helper = std::move(helper), aio_op = shared_from_this()]{
      aio_op->do_perform_may_block(*helper);
    });
    replace_running_cor_and_jump(*helper_ptr, std::move(work_cor));
    // After the previous function returns we know that fut_storage stores a future.
    assert(fut_storage);
    return std::move(fut_storage).get();
  }
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

template<typename T>
perform_helper<T>::perform_helper(opt_fut_T & fut)
 :fut_storage(fut)
{
  assert(!fut_storage);
}

template<typename T>
void 
perform_helper<T>::set_future(thr_queue::event::future<T> fut)
{
  if (fut_storage) {
    std::logic_error("future has already been set");
  }
  fut_storage = std::move(fut);
}

template<typename T>
bool
perform_helper<T>::future_already_set()
{
  return fut_storage.is_initialized();
}

/** \brief Implementation detail for make_aio_operation. Do not use.
 */
template <typename F>
class nonblock_lambda_aio_operation_t : public aio_operation_t<lambda_type_t<F>>
{
  using result_type = lambda_type_t<F>;
  using aiorf = typename aio_operation_t<result_type>::aio_result_future;
public:
  nonblock_lambda_aio_operation_t(F func)
    :function(std::move(func))
  {}
  ~nonblock_lambda_aio_operation_t() final override
  {
    this->perform_on_destruction_if_need();
  }

protected:
  aiorf do_perform_nonblock() final override
  {
    return function();
  }
  void do_perform_may_block(perform_helper<result_type> &helper) final override
  {
    abort();
  }
  bool may_block() final override
  {
    return false;
  }

private:
  F function;
};

/** \brief Implementation detail for make_aio_operation. Do not use.
 */
template <typename F>
class mayblock_lambda_aio_operation_t : public aio_operation_t<lambda_type_t<F>>
{
  using result_type = lambda_type_t<F>;
  using aiorf = typename aio_operation_t<result_type>::aio_result_future;
public:
  mayblock_lambda_aio_operation_t(F func)
    :function(std::move(func))
  {}
  ~mayblock_lambda_aio_operation_t() final override
  {
    this->perform_on_destruction_if_need();
  }

protected:
  aiorf do_perform_nonblock() final override
  {
    abort();
  }
  void do_perform_may_block(perform_helper<result_type> &helper) final override
  {
    function(helper);
  }
  bool may_block() final override
  {
    return true;
  }

private:
  F function;
};

template<typename F>
aio_operation<lambda_type_t<F>>
make_aio_operation(F function)
{
  using chosen_type = std::conditional_t<std::is_same<typename util::function_traits<F>::number_args,
    std::integral_constant<size_t, 0>>::value , nonblock_lambda_aio_operation_t<F>, mayblock_lambda_aio_operation_t<F>>;
  return std::make_shared<chosen_type>(std::move(function));
}
}
}
