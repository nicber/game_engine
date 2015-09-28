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
class worker_thread;

struct work_type_data {
  HANDLE iocp;
  const ULONG queue_completionkey = 0x1;
  moodycamel::ConcurrentQueue<coroutine> work_queue;
  moodycamel::ConcurrentQueue<coroutine> work_queue_prio;
  std::atomic<uint64_t> work_queue_size{0};
  std::atomic<uint64_t> work_queue_prio_size{0};
  std::atomic<unsigned int> waiting_threads{0};
  std::atomic<unsigned int> number_threads{0};
  std::atomic<bool> shutting_down{false};
};

class worker_thread {
public:
  worker_thread(work_type_data &dat);
  void loop();
  void do_work();
  void handle_io_operation(OVERLAPPED_ENTRY olapped_entry);
  ~worker_thread();

private:
  friend class global_thread_pool;
  work_type_data &data;
  std::atomic<bool> stopped;
  moodycamel::ConsumerToken ctok;
  moodycamel::ProducerToken ptok;
  moodycamel::ConsumerToken ctok_prio;
  moodycamel::ProducerToken ptok_prio;

public:
  boost::thread thr;
};

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
  work_type_data work_data;

  boost::mutex threads_mt;
  std::list<worker_thread> threads;
  const unsigned int hardware_concurrency;
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
