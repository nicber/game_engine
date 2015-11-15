#include "logging/log_priv.h"

namespace game_engine {
namespace logging {
class stdout_output : public output
{
public:
  void handle_complete_message(std::unique_ptr<message> me) override
  {
    std::ostringstream ss;
    ss << '[' << me->file << ": " << me->line << "] " << me->ss.str()
              << std::endl;
    printf("%s", ss.str().c_str());
  }
};

void handle_complete_message(std::unique_ptr<message> me)
{
  stdout_output output;
  output.handle_complete_message(std::move(me));
}
}
}
