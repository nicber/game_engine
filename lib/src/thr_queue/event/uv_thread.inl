#pragma once

#include <cassert>
#include "uv_thread.h"

namespace game_engine {
namespace thr_queue {
namespace event {
template <typename F>
boost::future<void>
uv_thr_sync_do(F func)
{
  auto &thr = get_uv_thr();
  assert(thr.global_async);
  assert(!thr.should_stop);
  boost::promise<void> prom;
  auto fut = prom.get_future();

  { 
    boost::lock_guard<boost::mutex> req_lock(thr.init_start_requests_mt);
    auto lambda = [func = std::move(func), prom = std::move(prom)] () mutable {
      try {
        func();
        prom.set_value();
      } catch (std::exception &e) {
        prom.set_exception(boost::copy_exception(e));
      }
    };

    auto functor_ptr = make_functor(std::move(lambda));
    thr.init_start_requests.emplace_back(std::move(functor_ptr));
  }

  uv_async_send(thr.global_async.get());
  return fut;
}
}
}
}