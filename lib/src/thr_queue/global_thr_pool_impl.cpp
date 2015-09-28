#include "global_thr_pool_impl.h"

#include <algorithm>
#include <iterator>
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
thread_local worker_thread *this_wthread = nullptr;

worker_thread::worker_thread(work_type_data &dat)
  :data(dat),
  stopped(false),
  ctok(data.work_queue),
  ptok(data.work_queue),
  ctok_prio(data.work_queue_prio),
  ptok_prio(data.work_queue_prio),
  thr([this] { loop(); }) {
}

void worker_thread::loop() {
  ++data.number_threads;
  try {
    this_wthread = this;
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
  stopped = true;
}

void worker_thread::do_work()
{
  bool could_work;
  do {
    could_work = false;

    coroutine work_to_do;

    if (run_next) {
      work_to_do = std::move(run_next.get());
      run_next = boost::none;
      goto do_work;
    }

    do {
      if (data.work_queue_prio.try_dequeue_from_producer(ptok_prio, work_to_do)
          || data.work_queue_prio.try_dequeue(work_to_do)) {
        --data.work_queue_prio_size;
        goto do_work;
      }
    } while (data.work_queue_prio_size > 0);

    do {
      if (data.work_queue.try_dequeue_from_producer(ptok, work_to_do)
          || data.work_queue.try_dequeue(work_to_do)) {
        --data.work_queue_size;
        goto do_work;
      }
      if (data.work_queue_prio.try_dequeue_from_producer(ptok_prio, work_to_do)
          || data.work_queue_prio.try_dequeue(work_to_do)) {
        --data.work_queue_prio_size;
        goto do_work;
      }
    } while (data.work_queue_size > 0);
    could_work = false;
    return;
    do_work:
    could_work = true;
    running_coroutine_or_yielded_from = &work_to_do;
    work_to_do.switch_to_from(*master_coroutine);
    if (*after_yield) {
      (*after_yield)();
      *after_yield = std::function<void()>();
    }
    running_coroutine_or_yielded_from = master_coroutine;
  } while (could_work);
}

void worker_thread::handle_io_operation(OVERLAPPED_ENTRY olapped_entry)
{
}

worker_thread::~worker_thread() {
  assert(stopped);
}

global_thread_pool::global_thread_pool()
  :hardware_concurrency(std::max(1u, boost::thread::hardware_concurrency()))
{
  auto c_threads = hardware_concurrency + 8;

  work_data.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, hardware_concurrency);

  boost::lock_guard<boost::mutex> lock(threads_mt);
  for (size_t i = 0; i < c_threads; ++i) {
    threads.emplace_back(work_data);
  }
}

global_thread_pool::~global_thread_pool()
{
  boost::unique_lock<boost::mutex> l(threads_mt);
  work_data.shutting_down = true;
  for (auto it = threads.begin(); it != threads.end();) {
    if (it->stopped) {
      continue;
      it = threads.erase(it);
    }
    // Wake up this specific thread.
    QueueUserAPC([](ULONG_PTR) {}, it->thr.native_handle(), 0);
    ++it;
  }
  while (threads.size()) {
    auto &thr = threads.back();
    l.unlock();
    thr.thr.join();
    l.lock();
    threads.pop_back();
  }
  assert(work_data.waiting_threads == 0);
  assert(work_data.number_threads == 0);
  CloseHandle(work_data.iocp);
}

void global_thread_pool::schedule(coroutine cor, bool first) {
  auto move_iter = std::make_move_iterator(&cor);
  schedule(move_iter, move_iter + 1, first);
}

void global_thread_pool::yield() {
  assert(running_coroutine_or_yielded_from != master_coroutine &&
         "we can't yield from the master_coroutine");
  master_coroutine->switch_to_from(*running_coroutine_or_yielded_from);
}

void global_thread_pool::yield_to(coroutine next)
{
  assert(!run_next);
  run_next = std::move(next);
  yield();
}

thread_local std::function<void()> *after_yield = nullptr;
thread_local coroutine *master_coroutine = nullptr;
thread_local coroutine *running_coroutine_or_yielded_from = nullptr;
thread_local boost::optional<coroutine> run_next;
global_thread_pool global_thr_pool;
}
}