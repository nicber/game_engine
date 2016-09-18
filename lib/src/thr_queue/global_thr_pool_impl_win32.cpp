#define BOOST_SCOPE_EXIT_CONFIG_USE_LAMBDAS
#include <boost/scope_exit.hpp>
#include "global_thr_pool_impl.h"
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
void
global_thread_pool::plat_wakeup_threads()
{
  auto ammount_work = work_data.work_queue_size + work_data.work_queue_prio_size;
  if (work_data.working_threads < ammount_work) {
    PostQueuedCompletionStatus(work_data.iocp, 0, work_data.queue_completionkey, nullptr);
  }
}

namespace platform {
worker_thread_impl::worker_thread_impl(work_data & dat)
 :data(dat)
{
}

void
worker_thread_impl::loop() {
  ++data.number_threads;
  try {
    this_wthread = (thr_queue::worker_thread*)(this);
    coroutine master_cor;
    global_thread_pool::after_yield_f after_yield_f;
    master_coroutine = &master_cor;
    after_yield = &after_yield_f;

    OVERLAPPED_ENTRY olapped_entry;

    auto wait_cond = [&] {
      while (true) {
        auto wait_time = data.shutting_down ? 0 : INFINITE;
        ULONG removed_entries;
        auto wait_iocp = GetQueuedCompletionStatusEx(data.iocp, &olapped_entry, 1, &removed_entries, wait_time, true);
        if (!wait_iocp) {
          auto err = GetLastError();
          if (err == WAIT_IO_COMPLETION) {
            continue;
          } else {
            if (err != WAIT_TIMEOUT) {
              LOG() << "GetQueuedCompletionStatus: " << err;
            }
            return false;
          }
        } else {
          break;
        }
      }
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
  get_internals().stopped = true;
}

void
worker_thread_impl::handle_io_operation(OVERLAPPED_ENTRY olapped_entry)
{
}

void
worker_thread_impl::wakeup()
{
  QueueUserAPC([](ULONG_PTR) {}, get_internals().thr.native_handle(), 0);
}

generic_work_data &
worker_thread_impl::get_data()
{
  return data;
}

work_data::work_data(unsigned int concurrency)
{
  iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrency);
}

work_data::~work_data()
{
  CloseHandle(iocp);
}
}
}
}
