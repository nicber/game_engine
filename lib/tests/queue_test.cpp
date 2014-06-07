#include "gtest/gtest.h"
#include "thr_queue/queue.h"
#include "thr_queue/global_thr_pool.h"

#include <chrono>
#include <thread>

TEST(ThrQueue, ExecutesCode) {
  using namespace game_engine::thr_queue;
  queue q(queue_type::parallel);
  std::atomic<int> count(0);

  for (size_t i = 0; i < 10000; ++i) {
    q.submit_work([&]() { count++; });
  }

  schedule_queue(std::move(q));

  // sleep to let the queue be executed.
  std::chrono::milliseconds dura(50);
  std::this_thread::sleep_for(dura);

  EXPECT_EQ(10000, count);
}
