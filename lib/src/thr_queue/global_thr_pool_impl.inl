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

template <typename InputIt>
void global_thread_pool::schedule(InputIt begin, InputIt end, bool first) {
  bool should_try_add = false;
  work_type_data *wdata = nullptr;
  std::list<worker_thread> *threads = nullptr;
  boost::mutex *thr_mt = nullptr;
  
  if(end - begin == 0) {
	return;
  static_assert(std::is_same<coroutine&&, decltype(*begin)>::value, "InputIt needs to move the coroutines");
  }

  auto cor_type = begin->type();
  assert(std::all_of(begin, end, [&](coroutine& cor) {
          return cor.type() == cor_type;
		}));
		
  if (cor_type == coroutine_type::io) {
    should_try_add = true;
    wdata = &io_data;
    threads = &io_threads;
    thr_mt = &io_threads_mt;
  } else {
    wdata = &cpu_data;
    threads = nullptr; // we never try to add more cpu threads
  }

  boost::unique_lock<boost::mutex> lock_wdata(wdata->mt);
  boost::unique_lock<boost::mutex> lock_thr_mt;
  if (thr_mt) {
    lock_thr_mt = boost::unique_lock<boost::mutex>(*thr_mt);
  }

  auto& queue_to_append_to = first ? wdata->work_queue_prio : wdata->work_queue;
  std::move(begin, end, std::back_inserter(queue_to_append_to));

  if (should_try_add && wdata->waiting_threads.size() == 0 &&
      threads->size() < max_io_threads) {
    threads->emplace_back(io_data);
  }

  if (wdata->waiting_threads.size()) {
    wdata->waiting_threads[0]->cv.notify_one();
  }
}
}
}
