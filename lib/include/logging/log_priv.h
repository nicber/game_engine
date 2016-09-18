#pragma once

#include "logging/log.h"
#include <memory>
#include <sstream>

namespace game_engine {
namespace logging {
struct message
{
  const char* function;
  const char* pretty_function;
  const char* file;
  unsigned int line;
  std::stringstream ss;
};

class output
{
public:
  virtual void handle_complete_message( std::unique_ptr< message > me ) = 0;
};

void handle_complete_message( std::unique_ptr< message > me );
}
}
