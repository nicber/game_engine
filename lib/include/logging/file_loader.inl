#include "file_loader.h"
#include "control_log.h"

namespace game_engine {
namespace logging {
template <typename It>
void load_policies_line_by_line(It begin, It end)
{
  for(auto it = begin; it != end; ++it) {
    std::string str = *it;
    try {
      load_policies_from_string(str);
    } catch (const parse_exception &e) {
      set_policy_for_file_line(__FILE__, __LINE__ + 1, policy::enable);
      LOG() << e.what();
    }
  }
}
}
}
