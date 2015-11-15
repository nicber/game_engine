#include "global_thr_pool_impl.h"

#include <algorithm>
#include <iterator>
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
worker_thread_internals::worker_thread_internals(generic_work_data & dat)
 :stopped(false),
  ctok(dat.work_queue),
  ptok(dat.work_queue),
  ctok_prio(dat.work_queue_prio),
  ptok_prio(dat.work_queue_prio)
{}

void
generic_worker_thread::start_thread()
{
  internals.emplace(get_data());
  internals->thr = boost::thread([this] { loop(); });
}

void
generic_worker_thread::schedule_coroutine(coroutine cor)
{
#ifdef _WIN32
  abort();
#else
  ++get_internals().thread_queue_size;
  get_internals().thread_queue.enqueue(std::move(cor));
  wakeup();
#endif
}

void
generic_worker_thread::do_work()
{
  bool could_work;
  int number_units_of_work = 0;
  do {
    could_work = false;

    coroutine work_to_do;

    if (run_next) {
      work_to_do = std::move(run_next.get());
      run_next = boost::none;
      goto do_work;
    }

    do {
      if (get_internals().thread_queue.try_dequeue(work_to_do)) {
        --get_internals().thread_queue_size;
        goto do_work;
      }
    } while (get_internals().thread_queue_size > 0);

    do {
      if (get_data().work_queue_prio.try_dequeue_from_producer(internals->ptok_prio, work_to_do)
          || get_data().work_queue_prio.try_dequeue(work_to_do)) {
        --get_data().work_queue_prio_size;
        goto do_work;
      }
    } while (get_data().work_queue_prio_size > 0);

    do {
      if (get_data().work_queue.try_dequeue_from_producer(internals->ptok, work_to_do)
          || get_data().work_queue.try_dequeue(work_to_do)) {
        --get_data().work_queue_size;
        goto do_work;
      }
      if (get_data().work_queue_prio.try_dequeue_from_producer(internals->ptok_prio, work_to_do)
          || get_data().work_queue_prio.try_dequeue(work_to_do)) {
        --get_data().work_queue_prio_size;
        goto do_work;
      }
    } while (get_data().work_queue_size > 0);
    could_work = false;
    break;
    do_work:
    ++number_units_of_work;
    could_work = true;
    running_coroutine_or_yielded_from = &work_to_do;
    work_to_do.switch_to_from(*master_coroutine);
    if (*after_yield) {
      (*after_yield)();
      *after_yield = std::function<void()>();
    }
    running_coroutine_or_yielded_from = master_coroutine;
  } while (could_work);
  if (number_units_of_work) 
    LOG() << number_units_of_work;
}

worker_thread_internals & 
generic_worker_thread::get_internals()
{
  assert(internals.is_initialized());
  return *internals;
}

worker_thread::worker_thread(work_data_combined & dat)
 :platform::worker_thread_impl(dat)
{
  start_thread();
}

worker_thread::~worker_thread()
{
  if (get_internals().thr.joinable()) {
    LOG() << "WARNING: Joining worker thread in its destructor.";
    get_internals().thr.join();
  }
  assert(get_internals().stopped);
}

global_thread_pool::global_thread_pool()
  :hardware_concurrency(std::max(1u, boost::thread::hardware_concurrency()))
  ,work_data(hardware_concurrency)
{
  auto c_threads = hardware_concurrency + 8;

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
    if (it->get_internals().stopped) {
      it->get_internals().thr.join();
      it = threads.erase(it);
      continue;
    }
    it->wakeup();
    ++it;
  }
  while (threads.size()) {
    auto &thr = threads.back();
    l.unlock();
    thr.get_internals().thr.join();
    l.lock();
    threads.pop_back();
  }
  assert(work_data.working_threads == 0);
  assert(work_data.number_threads == 0);
}

void global_thread_pool::schedule(coroutine cor, bool first) {
  auto move_iter = std::make_move_iterator(&cor);
  schedule(move_iter, move_iter + 1, first);
}

void global_thread_pool::yield() {
  assert(running_coroutine_or_yielded_from != nullptr);
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
thread_local worker_thread *this_wthread = nullptr;
global_thread_pool global_thr_pool;
}
}
