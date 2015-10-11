#pragma once

#include <atomic>
#include <thr_queue/thread_api.h>
#include <semaphore.h>
#include <set>
#include <sys/epoll.h>

namespace game_engine {
namespace thr_queue {
namespace platform {
class epoll_access_semaphore {
public:
  epoll_access_semaphore(unsigned int state);
  ~epoll_access_semaphore();

  void acquire();

  void release();

private:
  using thread_id_type = decltype(boost::this_thread::get_id());
  sem_t sem;
  std::set<thread_id_type> acquiring_threads;
  boost::mutex mt;
  const unsigned int initial_state;
};

struct work_data : generic_work_data {
  work_data(unsigned int concurrency);
  ~work_data();

  int epoll_fd;
  int wakeup_any_eventfd;
  epoll_access_semaphore semaphore;
  std::atomic<unsigned int> currently_polling{0};
};

class worker_thread_impl : public virtual base_worker_thread {
public:
  worker_thread_impl(work_data &dat);
  ~worker_thread_impl();

protected:
  void loop() override;
  void handle_io_operation(epoll_event epoll_ev);
  void please_die() override;
  generic_work_data &get_data() override;

private:
  work_data &data;
};

}
}
}
