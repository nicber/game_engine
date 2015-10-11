#pragma once

#include "global_thr_pool_impl.h"
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
namespace platform {
worker_thread_impl::worker_thread_impl(work_data & dat)
 :data(dat)
{
}

void
worker_thread_impl::loop() {
i/*  ++data.number_threads;
  try {
    this_wthread = static_cast<thr_queue::worker_thread*>(this);
    coroutine master_cor;
    std::function<void()> after_yield_f;
    master_coroutine = &master_cor;
    after_yield = &after_yield_f;

    OVERLAPPED_ENTRY olapped_entry;

    auto wait_cond = [&] {
      ++data.waiting_threads;

      while (true) {
        auto wait_time = data.shutting_down ? 0 : INFINITE;
        ULONG removed_entries;
        auto wait_iocp = GetQueuedCompletionStatusEx(data.iocp, &olapped_entry, 1, &removed_entries, wait_time, true);
        if (!wait_iocp) {
          auto err = GetLastError();
          if (err == WAIT_IO_COMPLETION) {
            continue;
          } else {
            --data.waiting_threads;
            if (err != WAIT_TIMEOUT) {
              LOG() << "GetQueuedCompletionStatus: " << err;
            }
            return false;
          }
        } else {
          break;
        }
      }
      --data.waiting_threads;
      return true;
    };

    do {
      if (olapped_entry.lpCompletionKey != data.queue_completionkey) {
        handle_io_operation(olapped_entry);
      } else {
        do_work();
      }
    } while (wait_cond());
  } catch (std::exception &e) {
    LOG() << "caught when worker thread was stopping: " << e.what();
  }
  --data.number_threads;
  get_internals().stopped = true;*/
}

void
worker_thread_impl::handle_io_operation(epoll_event epoll_ev)
{
}

void
worker_thread_impl::please_die()
{
}

generic_work_data &
worker_thread_impl::get_data()
{
  return data;
}

work_data::work_data(unsigned int concurrency)
{

}
}
}
}