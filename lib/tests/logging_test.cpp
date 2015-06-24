#include <gtest/gtest.h>
#include <logging/log.h>

using namespace game_engine::logging;

TEST(LoggingTest, HelloWorld)
{
  LOG() << "Hello World!";
}
