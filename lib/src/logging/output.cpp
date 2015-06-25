#include "logging/log_priv.h"

namespace game_engine {
namespace logging {
class stderr_output : public output
{
public:
  void handle_complete_message(std::unique_ptr<message> me) override
  {
    std::cerr << '[' << me->file << ": " << me->line << "] " << me->ss.str()
              << std::endl;
  }
};

void handle_complete_message(std::unique_ptr<message> me)
{
  stderr_output output;
  output.handle_complete_message(std::move(me));
}
}
}
