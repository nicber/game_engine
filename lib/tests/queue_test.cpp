#include "gtest/gtest.h"
#include "thr_queue/event/cond_var.h"
#include "thr_queue/queue.h"
#include "thr_queue/global_thr_pool.h"

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
  std::mutex mt;
  std::unique_lock<std::mutex> lock(mt);
  std::condition_variable cv;
  std::vector<int> counts;
  std::atomic<int> count(0);

  for (size_t i = 0; i < 100000; ++i) {
    queue q_par(queue_type::parallel);
    for (size_t i = 0; i < 10; ++i) {
      q_par.submit_work([&] { count++; });
    }

    q_ser.append_queue(std::move(q_par));
    q_ser.submit_work([&] {
      std::lock_guard<std::mutex> lock(mt);
      counts.push_back(count);
      count = 0;
    });
  }

  q_ser.submit_work([&] {
    std::lock_guard<std::mutex> lock(mt);
    cv.notify_one();
  });

  schedule_queue(std::move(q_ser));

  cv.wait(lock);

  for (auto i : counts) {
    EXPECT_EQ(1, i);
  }
}
