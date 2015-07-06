#pragma once

#include <iostream>
#include <memory>

namespace game_engine {
namespace logging {
bool enabled(const char *file, unsigned int line);

struct message;

class logger
{
public:
  logger(const char *function, const char *pretty_function,
         const char *file, unsigned int line);

  ~logger();

  template <typename T>
  std::ostream &operator<<(const T &rhs)
  {
    return get_stream() << rhs;
  }

  std::ostream &get_stream();

private:
  std::unique_ptr<message> d;
};

#define LOG() if (game_engine::logging::enabled(__FILE__, __LINE__)) game_engine::logging::logger(__FUNCTION__, __PRETTY_FUNCTION__, __FILE__, __LINE__)
}
}
