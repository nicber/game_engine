#include "gtest/gtest.h"
#include "thr_queue/queue.h"

TEST(ThrQueue, ExecutesCode) {
  using namespace game_engine::thr_queue;
  queue q(queue_type::parallel);
  std::atomic<int> count(0);

  for (size_t i = 0; i < 10; ++i) {
                q.submit_work([&]() { count++; });
  }

  q.run_in_pool(block::yes, 1);
  EXPECT_EQ(10, count);
}
