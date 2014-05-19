#include "memory/vers_ptr.h"
#include "gtest/gtest.h"

#include <deque>

using namespace game_engine;

struct test_pol {
  static bool must_copy_val;
  static std::deque<base_vers_data::time_point> times;

  bool must_copy(base_vers_data::time_point) { return must_copy_val; }

  base_vers_data::time_point get_new_time() {
    auto ret = times.front();
    times.pop_front();
    return ret;
  }
};

bool test_pol::must_copy_val = false;
std::deque<base_vers_data::time_point> test_pol::times;

using int_vers_data = vers_data<int, test_pol>;

TEST(Memory, CopiesWhenMust) {
  test_pol::must_copy_val = true;
  test_pol::times.push_back(10);
  test_pol::times.push_back(11);

  int_vers_data data_test(1);
  auto data_ptr = data_test.ptr();

  *data_ptr = 3;
  test_pol::must_copy_val = false;

  EXPECT_EQ( 3,*data_ptr);
  EXPECT_EQ(3, data_test.get_value_of(0u));
  EXPECT_EQ(1, data_test.get_value_of(1u));

  EXPECT_EQ(11, data_test.get_time_of(0u));
  EXPECT_EQ(10, data_test.get_time_of(1u));

  EXPECT_EQ(data_test.get_latest(), *data_ptr);

  EXPECT_EQ(2u, data_test.size());
}

TEST(Memory, RemoveOlderNewer) {
  test_pol::must_copy_val = true;
  test_pol::times.push_back(10);
  test_pol::times.push_back(11);
  test_pol::times.push_back(12);
  test_pol::times.push_back(13);
  test_pol::times.push_back(14);

  int_vers_data data_test(1);
  auto data_ptr = data_test.ptr();

  for (size_t i = 0; i < 4; ++i) {
    *data_ptr = i;
  }

  EXPECT_EQ(5u, data_test.size());

  data_test.remove_older_than(12);

  EXPECT_EQ(3u, data_test.size());

  data_test.remove_newer_than(12);

  EXPECT_EQ(1u, data_test.size());

  EXPECT_THROW({ data_test.remove_older_than(13); }, std::runtime_error);
  EXPECT_THROW({ data_test.remove_newer_than(11); }, std::runtime_error);
}
