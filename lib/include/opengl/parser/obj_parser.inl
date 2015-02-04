#include "obj_parser.h"

namespace game_engine {
namespace opengl {
namespace parser {
template <typename It>
obj_model parse_obj_format(It begin, It end) {
  auto dist = std::distance(begin, end);
  std::vector<char> buffer(dist + sizeof('\0'));
  std::copy(begin, end, std::back_inserter(buffer));
  buffer.emplace_back('\0'); // just in case
  return parse_obj_format(buffer.data());
}
}
}
}
