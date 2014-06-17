#include <algorithm>
#include "gtest/gtest.h"
#include "thr_queue/event/cond_var.h"
#include "thr_queue/event/mutex.h"
#include "thr_queue/global_thr_pool.h"
#include "thr_queue/queue.h"
#include "thr_queue/util_queue.h"

#include <chrono>
#include <thread>

TEST(ThrQueue, ExecutesCode) {
  using namespace game_engine::thr_queue;
  queue q(queue_type::parallel);
  std::atomic<int> count(0);

  for (size_t i = 0; i < 10; ++i) {
    q.submit_work([&]() { count++; });
  }

  schedule_queue(std::move(q));

  // sleep to let the queue be executed.
  std::chrono::milliseconds dura(50);
  std::this_thread::sleep_for(dura);

  EXPECT_EQ(10, count);
}

TEST(ThrQueue, ParallelQinSerial) {
  using namespace game_engine::thr_queue;
  queue q_ser(queue_type::serial);
  queue q_par(queue_type::parallel);
  std::atomic<int> count(0);

  for (size_t i = 0; i < 10; ++i) {
    q_par.submit_work([&] { count++; });
  }

  q_ser.append_queue(std::move(q_par));

  auto fut = q_ser.submit_work([] {});

  schedule_queue(std::move(q_ser));

  fut.wait();

  EXPECT_EQ(10, count);
}

TEST(ThrQueue, SerialQinParallel) {
  using namespace game_engine::thr_queue;
  queue q_par(queue_type::parallel);
  std::mutex mt;
  std::condition_variable cv;
  std::atomic<int> count[100];
  std::vector<int> counts[100];
  std::atomic<int> done(0);

  std::generate(std::begin(count), std::end(count), [] { return 0; });

  for (size_t i = 0; i < 100; ++i) {
    queue q_ser(queue_type::serial);

    for (size_t j = 0; j < 100; ++j) {
      q_ser.submit_work([&, i] {
        auto val = ++count[i];
        counts[i].push_back(val);
      });
    }

    q_ser.submit_work([&] {
      ++done;
      std::lock_guard<std::mutex> lock(mt);
      cv.notify_one();
    });

    q_par.append_queue(std::move(q_ser));
  }

  std::unique_lock<std::mutex> lock(mt);

  schedule_queue(std::move(q_par));

  while (done < 100) {
    cv.wait(lock);
  }

  for (int val : count) {
    EXPECT_EQ(100, val);
  }

  for (auto &vec : counts) {
    int prev_step = 0;
    for (int step : vec) {
      EXPECT_EQ(prev_step + 1, step);
      prev_step = step;
    }
  }
}

TEST(ThrQueue, DefaultParQueue) {
  std::atomic<int> count(0);
  std::atomic<bool> done(false);
  std::mutex mt;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(mt);

  for (size_t i = 0; i < 10000; ++i) {
    game_engine::thr_queue::default_par_queue().submit_work([&] {
      if (++count == 10000) {
        std::lock_guard<std::mutex> lock(mt);
        done = true;
        cv.notify_one();
      }
    });
  }

  while (!done) {
    cv.wait(lock);
  }

  EXPECT_EQ(10000, count);
}

TEST(ThrQueue, SerQueue) {
  auto q_ser = game_engine::thr_queue::ser_queue();
  // we use atomics to be sure that a possible bug in the implementation isn't
  // "hiding" itself by means of a race condition.
  std::atomic<int> last_exec(-1);
  std::atomic<bool> correct_order(true);

  for (int i = 0; i < 10000; ++i) {
    q_ser.submit_work([&, i] {
      if (last_exec.exchange(i) != i - 1) {
        correct_order = false;
      }
    });
  }

  EXPECT_EQ(9999, last_exec);
  EXPECT_EQ(true, correct_order);
}

TEST(ThrQueue, LockUnlock) {
  using e_mutex = game_engine::thr_queue::event::mutex;
  using game_engine::thr_queue::queue;
  using game_engine::thr_queue::queue_type;
  queue q_par(queue_type::parallel);

  e_mutex mt;
  bool done = false;
  std::mutex cv_mt;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(cv_mt);

  struct {
    int a = 0;
    int b = 0;
  } test;

  for (size_t i = 0; i < 10000; i++) {
    q_par.submit_work([&] {
      std::lock_guard<e_mutex> lock(mt);
      test.a++;
      test.b++;
    });
  }

  queue q_ser(queue_type::serial);
  q_ser.append_queue(std::move(q_par));
  q_ser.submit_work([&] {
    std::lock_guard<std::mutex> lock_lamb(cv_mt);
    done = true;
    cv.notify_one();
  });

  schedule_queue(std::move(q_ser));

  cv.wait(lock, [&] { return done; });

  EXPECT_EQ(10000, test.a);
  EXPECT_EQ(10000, test.b);
}
