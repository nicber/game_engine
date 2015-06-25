#include <functional>
#include "logging/log.h"
#include <string>

namespace game_engine {
namespace logging {
enum class policy {
  enable,
  disable
};

void set_default_for_all(policy pol);
void set_default_for_file(std::string file, policy pol);
void set_policy_for_file_line(std::string file, unsigned int line,
                              policy pol);

policy get_policy_for(const std::string &file, unsigned int line);
policy get_policy_for_file(const std::string &file);
policy get_default_policy();

void remove_file_policy(const std::string &file);
void remove_file_line_policy(const std::string &file, unsigned int line);

struct applied_policy {
  const std::string &file;
  unsigned int line;
  policy pol;

  applied_policy(const std::string &f, unsigned int l, policy p);
};

using visitor_func = std::function<void(applied_policy)>;

void apply_to_all_policies(visitor_func);
}
}
