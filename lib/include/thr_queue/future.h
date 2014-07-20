#pragma once

#include "thr_queue/thread_api.h"
#include <utility>

namespace game_engine {
namespace thr_queue {
template <typename F>
using get_future_type = boost::future<decltype(std::declval<F>()())>;

template <typename F>
using get_promise_type = boost::promise<decltype(std::declval<F>()())>;
}
}
