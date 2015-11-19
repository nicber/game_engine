#pragma once

#include <algorithm>
#include "global_thr_pool_impl.h"
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
template <typename F>
void global_thread_pool::yield(F func) {
  *after_yield = std::function<void()>(std::move(func));
  yield();
}

template<typename F>
void global_thread_pool::yield_to(coroutine next, F after_yield)
{
  assert(!run_next);
  run_next = std::move(next);
  yield(std::move(after_yield));
}

template <typename InputIt>
void global_thread_pool::schedule(InputIt begin, InputIt end, bool first) {
  static_assert(std::is_same<coroutine&&, decltype(*begin)>::value, "InputIt needs to move the coroutines");

  auto count = std::distance(begin, end);
  if (count == 0) {
  	return;
  }

#ifndef _WIN32
  std::vector<coroutine> tmp_buffer;
  tmp_buffer.reserve(count);

  for (auto it = begin; it != end; ++it) {
    auto cor = *it;
    if (cor.bound_thread) {
      auto *thr = cor.bound_thread;
      thr->schedule_coroutine(std::move(cor));
    } else {
      tmp_buffer.emplace_back(std::move(cor));
    }
  }

  auto tmp_begin = std::make_move_iterator(tmp_buffer.begin());
  auto tmp_size = tmp_buffer.size();
#else 
  auto tmp_begin = begin;
  auto tmp_size = count;
#endif

  if (this_wthread) {
    if (first) {
      work_data.work_queue_prio_size += tmp_size;
      work_data.work_queue_prio.enqueue_bulk(this_wthread->get_internals().ptok_prio, tmp_begin, tmp_size);
    } else {
      work_data.work_queue_size += tmp_size;
      work_data.work_queue.enqueue_bulk(this_wthread->get_internals().ptok, tmp_begin, tmp_size);
    }
  } else {
    if (first) {
      work_data.work_queue_prio_size += tmp_size;
      work_data.work_queue_prio.enqueue_bulk(tmp_begin, tmp_size);
    } else {
      work_data.work_queue_size += tmp_size;
      work_data.work_queue.enqueue_bulk(tmp_begin, tmp_size);
    }
  }

  plat_wakeup_threads(tmp_size);
}
}
}
