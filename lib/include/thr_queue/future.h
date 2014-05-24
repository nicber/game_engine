#pragma once

#include <future>
#include <utility>

namespace game_engine {
namespace thr_queue {
template <typename F>
using get_future_type = std::future<decltype(std::declval<F>()())>;

template <typename F>
using get_promise_type = std::promise<decltype(std::declval<F>()())>;
}
}
