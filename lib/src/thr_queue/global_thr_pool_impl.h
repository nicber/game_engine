#pragma once

#include <atomic>
#include <cassert>
#include <concurrentqueue.h>
#include <list>
#include <memory>
#include <stack>
#include "thr_queue/coroutine.h"
#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
struct generic_work_data {
  moodycamel::ConcurrentQueue<coroutine> work_queue;
  moodycamel::ConcurrentQueue<coroutine> work_queue_prio;
  std::atomic<uint64_t> work_queue_size{ 0 };
  std::atomic<uint64_t> work_queue_prio_size{ 0 };
  std::atomic<unsigned int> working_threads{ 0 };
  std::atomic<unsigned int> number_threads{ 0 };
  std::atomic<bool> shutting_down{ false };
};

struct worker_thread_internals {
  worker_thread_internals(generic_work_data &dat);

  std::atomic<bool> stopped;
  moodycamel::ConsumerToken ctok;
  moodycamel::ProducerToken ptok;
  moodycamel::ConsumerToken ctok_prio;
  moodycamel::ProducerToken ptok_prio;
  moodycamel::ConcurrentQueue<coroutine> thread_queue;
  std::atomic<unsigned int> thread_queue_size;
  boost::thread thr;
};

class base_worker_thread {
protected:
  virtual void loop() = 0;
  virtual void wakeup() = 0;
  virtual void do_work() = 0;
  virtual void start_thread() = 0;
  virtual generic_work_data &get_data() = 0;
  virtual worker_thread_internals &get_internals() = 0;
};
}
}

#ifdef _WIN32
#include "global_thr_pool_impl_win32.h"
#else
#include "global_thr_pool_impl_linux.h"
#endif

namespace game_engine {
namespace thr_queue {
using work_data_combined = platform::work_data;

class generic_worker_thread : public virtual base_worker_thread {
protected:
  void do_work() final override;
  worker_thread_internals &get_internals() final override;
  void start_thread() final override;
  void schedule_coroutine(coroutine cor);

private:
  boost::optional<worker_thread_internals> internals;
};

#pragma warning( push )
#pragma warning( disable : 4250 )
class worker_thread final : protected generic_worker_thread, protected platform::worker_thread_impl
{
public:
  worker_thread(work_data_combined &dat);
  ~worker_thread();

  using generic_worker_thread::get_internals;
  using generic_worker_thread::schedule_coroutine;
  using platform::worker_thread_impl::wakeup;
};
#pragma warning( pop )

class global_thread_pool {
public:
  global_thread_pool();
  ~global_thread_pool();

  void schedule(coroutine cor, bool first);
  
  template <typename InputIt>
  void schedule(InputIt begin, InputIt end, bool first);
  
  void yield();

  template <typename F>
  void yield(F func);

  void yield_to(coroutine next);

  template <typename F>
  void yield_to(coroutine next, F after_yield);

private:
  void plat_wakeup_threads(unsigned int count);

private:
  boost::mutex threads_mt;
  std::list<worker_thread> threads;
  const unsigned int hardware_concurrency;
  work_data_combined work_data;
};

extern global_thread_pool global_thr_pool;

extern thread_local std::function<void()> *after_yield;
extern thread_local coroutine *master_coroutine;
extern thread_local coroutine *running_coroutine_or_yielded_from;
extern thread_local boost::optional<coroutine> run_next;
extern thread_local worker_thread *this_wthread;
}
}

#include "global_thr_pool_impl.inl"
