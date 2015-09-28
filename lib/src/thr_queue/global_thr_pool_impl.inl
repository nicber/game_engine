#pragma once

#include <algorithm>
#include "global_thr_pool_impl.h"

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

  auto cor_type = begin->type();
  assert(std::all_of(begin, end, [&](coroutine& cor) {
          return cor.type() == cor_type;
		}));
		
  if (this_wthread) {
    if (first) {
      work_data.work_queue_prio_size += count;
      work_data.work_queue_prio.enqueue_bulk(this_wthread->ptok_prio, std::move(begin), count);
    } else {
      work_data.work_queue_size += count;
      work_data.work_queue.enqueue_bulk(this_wthread->ptok, std::move(begin), count);
    }
  } else {
    if (first) {
      work_data.work_queue_prio_size += count;
      work_data.work_queue_prio.enqueue_bulk(std::move(begin), count);
    } else {
      work_data.work_queue_size += count;
      work_data.work_queue.enqueue_bulk(std::move(begin), count);
    }
  }

  decltype(count) i = 0;
  while(true) {
    unsigned int working = work_data.number_threads - work_data.waiting_threads;
    if (working < hardware_concurrency && i < count) {
      PostQueuedCompletionStatus(work_data.iocp, 0, work_data.queue_completionkey, nullptr);
      ++i;
    } else {
      break;
    }
  }
}
}
}
