#define BOOST_SCOPE_EXIT_CONFIG_USE_LAMBDAS
#include <boost/scope_exit.hpp>
#include "global_thr_pool_impl.h"
#include <logging/log.h>
#include <poll.h>
#include <signal.h>
#include <sys/eventfd.h>

namespace game_engine {
namespace thr_queue {
namespace platform {
worker_thread_impl::worker_thread_impl(work_data & dat)
 :data(dat)
{
}

worker_thread_impl::~worker_thread_impl()
{
}

void
worker_thread_impl::loop() {
  static int set_usr2_sigaction = [] {
    struct sigaction sigact;
    sigact.sa_handler = [] (int) {};
    sigact.sa_flags = 0;
    if (sigemptyset(&sigact.sa_mask)) {
      std::ostringstream ss;
      ss << "sigemptyset failed: " << strerror(errno);
      LOG() << ss.str();
      throw std::runtime_error(ss.str());
    }
    if(sigaction(SIGHUP, &sigact, nullptr)) {
      std::ostringstream ss;
      ss << "sigaction failed: " << strerror(errno);
      LOG() << ss.str();
      throw std::runtime_error(ss.str());
    }
    return 0;
  }();
  assert(set_usr2_sigaction == 0);
  sigset_t usr2_blocked_set, original_set;
  if (sigemptyset(&usr2_blocked_set) || sigemptyset(&original_set)) {
    std::ostringstream ss;
    ss << "sigemptyset failed: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
  if (sigaddset(&usr2_blocked_set, SIGHUP)) {
    std::ostringstream ss;
    ss << "sigaddset failed: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
  pthread_sigmask(SIG_BLOCK, &usr2_blocked_set, &original_set);

  ++data.number_threads;
  try {
    this_wthread = (thr_queue::worker_thread*)(this);
    coroutine master_cor;
    std::function<void()> after_yield_f;
    master_coroutine = &master_cor;
    after_yield = &after_yield_f;

    epoll_event epoll_entry;

    auto wait_cond = [&] {
      memset(&epoll_entry, 0, sizeof(epoll_entry));
      data.semaphore.acquire();
      int epoll_ret;
      while(true) {
        if (get_internals().thread_queue_size > 0) {
          epoll_entry.data.ptr = &data.wakeup_any_eventfd;
          return true;
        }
        auto wait_time = data.shutting_down ? 0 : 1000;
        epoll_ret = epoll_pwait(data.epoll_fd, &epoll_entry, 1, wait_time,
                                &original_set);
        if (epoll_ret == 0) {
          if (wait_time == 0) {
            return false;
          } else  {
            epoll_entry.data.ptr = &data.wakeup_any_eventfd;
            return true;
          }
        } else if (epoll_ret == 1) { //success
          break;
        } else if (errno == EINTR) {
          epoll_entry.data.ptr = &data.wakeup_any_eventfd;
          LOG() << "recv HUP " << boost::this_thread::get_id();
          return true;
        } else {
          std::ostringstream ss;
          ss << "Error epolling: " << strerror(errno);
          LOG() << ss.str();
          return false;
        }
      }

      if (epoll_entry.data.ptr == &data.wakeup_any_eventfd) {
        eventfd_t val;
        int efd_read_ret = eventfd_read(data.wakeup_any_eventfd, &val);
        LOG() << boost::this_thread::get_id() << ' ' << val;

        BOOST_SCOPE_EXIT(this) {
          epoll_event readd_wakeup_epoll;
          readd_wakeup_epoll.events = EPOLLIN | EPOLLONESHOT;
          readd_wakeup_epoll.data.ptr = &data.wakeup_any_eventfd;
          int epoll_ctl_ret = epoll_ctl(data.epoll_fd, EPOLL_CTL_MOD, data.wakeup_any_eventfd,
                                        &readd_wakeup_epoll);
          if (epoll_ctl_ret == -1) {
            std::ostringstream ss;
            ss << "Error readding wakeup_any_eventfd to global epoll: " << strerror(errno);
            LOG() << ss.str();
            throw std::runtime_error(ss.str());
          }
        };

        if (efd_read_ret != 0) {
          std::ostringstream ss;
          ss << "Error when decreasing the wakeup_any_eventfd counter: " << strerror(errno);
          LOG() << ss.str();
          throw std::runtime_error(ss.str());
        }
      }

      return true;
    };

    while (wait_cond()) {
      if (epoll_entry.data.ptr != &data.wakeup_any_eventfd) {
        handle_io_operation(epoll_entry);
      } else {
        do_work();
      }
    }
  } catch (std::exception &e) {
    LOG() << "caught when worker thread was stopping: " << e.what();
  }
  data.semaphore.release();
  --data.number_threads;
  get_internals().stopped = true;
}

void
worker_thread_impl::handle_io_operation(epoll_event)
{
  abort();
}

void
worker_thread_impl::wakeup()
{
  if (int error = pthread_kill(get_internals().thr.native_handle(), SIGHUP)) {
    LOG() << "pthread_kill failed: " << strerror(error);
  }
  LOG() << "HUP " << get_internals().thr.get_id();
}

generic_work_data &
worker_thread_impl::get_data()
{
  return data;
}

work_data::work_data(unsigned int concurrency)
 :concurrency_max(concurrency)
 ,semaphore(concurrency)
{
  epoll_fd = epoll_create(1);
  if (epoll_fd == -1) {
    std::ostringstream ss;
    ss << "Error creating global epoll fd: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
  wakeup_any_eventfd = eventfd(0, 0);
  if (wakeup_any_eventfd == -1) {
    std::ostringstream ss;
    ss << "Error creating wakeup_any_eventfd: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
  epoll_event add_wakeup_epoll;
  add_wakeup_epoll.events = EPOLLIN | EPOLLONESHOT;
  add_wakeup_epoll.data.ptr = &wakeup_any_eventfd;
  int epoll_ctl_ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wakeup_any_eventfd,
                                &add_wakeup_epoll);
  if (epoll_ctl_ret == -1) {
    std::ostringstream ss;
    ss << "Error adding wakeup_any_eventfd to global epoll: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
}

work_data::~work_data()
{
  int close_ret;
  while (true) {
    close_ret = close(epoll_fd);
    if (close_ret == 0) {
      break;
    } else if (errno == EINTR) {
      continue;
    } else {
      std::ostringstream ss;
      ss << "Error closing global epoll: " << strerror(errno);
      LOG() << ss.str();
      break;
    }
  }
}

epoll_access_semaphore::epoll_access_semaphore(unsigned int state)
 :initial_state(state)
{
  int sem_init_ret = sem_init(&sem, 0, state);
  if (sem_init_ret != 0) {
    std::ostringstream ss;
    ss << "Error constructing semaphore: " << strerror(errno);
    LOG() << ss.str();
    throw std::runtime_error(ss.str());
  }
}

epoll_access_semaphore::~epoll_access_semaphore()
{
  int sem_val;
  sem_getvalue(&sem, &sem_val);
  if ((unsigned int) sem_val != initial_state) {
    LOG() << "Warning: destructing semaphore with state that differs from "
             "initial one: " << "current = " << sem_val << ", initial = "
          << initial_state;
  }
  int sem_destroy_ret = sem_destroy(&sem);
  assert (sem_destroy_ret == 0);
  // sem_destroy only fails if sem is not a semaphore.
}

void
epoll_access_semaphore::acquire()
{
  boost::unique_lock<boost::mutex> lock(mt);
  if (acquiring_threads.count(boost::this_thread::get_id()) == 0) {
    lock.unlock();
    while (true) {
      int sem_wait_ret = sem_wait(&sem);
      if (sem_wait_ret == 0) { //success
        break;
      } else if (errno == EINTR) {
        continue;
      } else {
        std::ostringstream ss;
        ss << "Error waiting on semaphore: " << strerror(errno);
        LOG() << ss.str();
        throw std::runtime_error(ss.str());
      }
    }
    lock.lock();
    auto emplace_res = acquiring_threads.emplace(boost::this_thread::get_id());
    assert(emplace_res.second);
  } else {
    return;
  }
}

void
epoll_access_semaphore::release()
{
  boost::unique_lock<boost::mutex> lock(mt);
  if (acquiring_threads.count(boost::this_thread::get_id()) == 1) {
    int sem_post_ret = sem_post(&sem);
    assert(sem_post_ret == 0);
    auto erase_ret = acquiring_threads.erase(boost::this_thread::get_id());
    assert(erase_ret == 1);
  } else {
    LOG() << "Warning: a thread that has not acquired the semaphore has tried to release it";
  }
}
}

void
global_thread_pool::plat_wakeup_threads()
{
  auto ammount_work = work_data.work_queue_size + work_data.work_queue_prio_size;
  if (work_data.working_threads < ammount_work) {
    int write_ret = eventfd_write(work_data.wakeup_any_eventfd, 1);
    LOG() << "Wrote to eventfd";
    if (write_ret != 0) {
      std::ostringstream ss;
      ss << "Error when writing to eventfd to wakeup a thread: " << strerror(errno);
      LOG() << ss.str();
      throw std::runtime_error(ss.str());
    }
  }
}
}
}
