#include <atomic>
#include <condition_variable>
#include <iterator>
#include <thread>

#include "thr_queue/queue.h"
#include "thr_queue/thread_pool.h"

namespace game_engine {
namespace thr_queue {
void swap(queue &lhs, queue &rhs) {
  std::lock(rhs.queue_mut, rhs.queue_mut);
  std::lock_guard<std::recursive_mutex> lock_lhs(lhs.queue_mut,
                                                 std::adopt_lock);
  std::lock_guard<std::recursive_mutex> lock_rhs(rhs.queue_mut,
                                                 std::adopt_lock);

  using std::swap;
  swap(lhs.work_queue, rhs.work_queue);
  swap(lhs.typ, rhs.typ);
}

queue::queue(queue &&rhs) : typ(queue_type::parallel) { swap(*this, rhs); }

queue &queue::operator=(queue &&rhs) {
  this->~queue();
  new (this) queue(std::move(rhs));
  return *this;
}

struct ser_in_par_work : queue::base_work {
  ser_in_par_work(queue qu) : q(std::move(qu)) {}
  virtual void operator()() final override { q.run_until_empty(); }

  queue q;
};

struct par_in_ser_work : queue::base_work {
  par_in_ser_work(queue qu) : q(std::move(qu)) {}
  virtual void operator()() final override {
    get_thread_pool().schedule_queue_first(std::move(q));
  }

  queue q;
};

queue::queue(queue_type ty) : typ(ty) {}
void queue::submit_queue(queue q) {
  std::lock(queue_mut, q.queue_mut);
  std::lock_guard<std::recursive_mutex> my_lock(queue_mut, std::adopt_lock);
  std::lock_guard<std::recursive_mutex> q_lock(q.queue_mut, std::adopt_lock);

  if (typ == q.typ) {
    std::copy(std::make_move_iterator(q.work_queue.begin()),
              std::make_move_iterator(q.work_queue.end()),
              std::back_inserter(work_queue));
  } else if (typ == queue_type::parallel && q.typ == queue_type::serial) {
    work_queue.emplace_back(new ser_in_par_work(std::move(q)));
  } else if (typ == queue_type::serial && q.typ == queue_type::parallel) {
    work_queue.emplace_back(new par_in_ser_work(std::move(q)));
  }
}

bool queue::run_once() {
  std::unique_lock<std::recursive_mutex> lock(queue_mut);

  if (work_queue.size() == 0) {
    return false;
  }
  auto work = std::move(work_queue.front());
  work_queue.pop_front();

  // we want to allow another thread to call this member function while we are
  // running the work unit.
  if (typ == queue_type::parallel) {
    lock.unlock();
  }
  (*work)();
  return true;
}

void queue::run_until_empty() {
  std::lock_guard<std::recursive_mutex> lock(queue_mut);

  while (work_queue.size() > 0) {
    auto work = std::move(work_queue.front());
    work_queue.pop_front();
    (*work)();
  }
}

void run_queue_in_pool_impl(bool add_this_thread_if_block, block bl, queue q,
                            std::unique_ptr<queue::base_work> last_work,
                            thread_pool &pool) {
  if (bl == block::yes || last_work) {
    queue q_ser =
        q.typ == queue_type::serial ? std::move(q) : queue(queue_type::serial);

    if (q.typ == queue_type::parallel) {
      q_ser.submit_queue(std::move(q));
    }

    if (bl == block::yes) {
      // block: yes; last_work: both
      std::mutex mt;
      std::unique_lock<std::mutex> lock(mt);
      std::condition_variable cv;
      std::atomic<bool> finished(false);

      q_ser.submit_work([&]() {
        finished = true;
        // since the monitoring thread unlocks the mutex when it starts
        // waiting, we can only lock it when it's already waiting.
		// this way we can't notify when it's not yet waiting, and we avoid
		// deadlocking the monitoring thread.
        // this is a pessimization according to cppreference.
        // http://en.cppreference.com/w/cpp/thread/condition_variable/notify_one
        std::unique_lock<std::mutex> lock(mt);
        lock.unlock();
        cv.notify_one();
      });

      if (last_work) {
        q_ser.work_queue.push_back(std::move(last_work));
      }

      pool.schedule_queue(std::move(q_ser));
      if (add_this_thread_if_block) {
		lock.unlock();
        pool.participate(kickable::no);
		lock.lock();
      }

	  // even though finished is an atomic not protected by the lock,
	  // if it is set to true after this check we wont wait indefinitely because
	  // the thread that set it to true has to wait until we start waiting.
	  if(finished == false) {
      	cv.wait(lock, [&]() { return finished == true; });
	  }
      return;
    } else {
      // block: no; last_work: yes
      q_ser.work_queue.push_back(std::move(last_work));
      pool.schedule_queue(std::move(q_ser));
      return;
    }
  } else {
    // block: no; last_work: no
    pool.schedule_queue(std::move(q));
    return;
  }
}
void queue::run_in_pool(block bl, unsigned int max_pool_size) {
  if (max_pool_size == 0) {
    throw std::invalid_argument("max_pool_size == 0");
  }

  // instead of waiting this thread will paaticipate in the thread pool.
  if (bl == block::yes) {
    max_pool_size--;
  }

  auto ptr_thr_pool =
      std::unique_ptr<thread_pool>(new thread_pool(max_pool_size));
  auto &thr_pool_ref = *ptr_thr_pool;

  std::unique_lock<std::recursive_mutex> lock(queue_mut);
  auto moved_queue = std::move(*this);
  *this = queue(moved_queue.typ);
  lock.unlock();

  // we run the thread pools destructor after removing this thread from it,
  // causing every other thread to finish what it's doing and be destroyed.

  struct last_work : base_work {
    void operator()() override final {
    }
    std::unique_ptr<thread_pool> thr_pool;
  };

  auto last_work_ptr = std::unique_ptr<last_work>(new last_work);
  last_work_ptr->thr_pool = std::move(ptr_thr_pool);

  run_queue_in_pool_impl(true, bl, std::move(moved_queue),
                         std::move(last_work_ptr), thr_pool_ref);
}

void queue::run_in_pool(block bl, thread_pool &pool) {
  std::unique_lock<std::recursive_mutex> lock(queue_mut);

  auto moved_queue = std::move(*this);
  *this = queue(moved_queue.typ);
  lock.unlock();

  run_queue_in_pool_impl(false, bl, std::move(moved_queue), nullptr, pool);
}

queue_type queue::type() const { return typ; }
}
}
