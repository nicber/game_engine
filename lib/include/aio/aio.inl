#pragma once

#include "aio.h"

namespace game_engine {
namespace aio {
template<typename T>
aio_result_future<T>
game_engine::aio::aio_operation_t<T>::perform()
{
  assert(!already_performed);
  already_performed = true;
  return do_perform();
}

template<typename T>
game_engine::aio::aio_operation_t<T>::~aio_operation_t()
{
  if (get_perform_on_destruction() && !already_performed) {
    perform();
  }
}

}
}
