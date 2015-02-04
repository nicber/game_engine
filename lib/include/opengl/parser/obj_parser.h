#pragma once

#include <stdexcept>
#include <glm/vec3.hpp>
#include <vector>

namespace game_engine {
namespace opengl {
namespace parser {
class parse_error : public std::runtime_error {
public:
  parse_error(std::string message_str_, std::string source_str_, size_t pos_);

private:
  std::string message_str;
  std::string source_str;
  size_t pos;
};

struct obj_model {
  std::vector<unsigned int> indices;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> tex_coords;
  std::vector<glm::vec3> normals;
};

template <typename It>
obj_model parse_obj_format(It begin, It end);

obj_model parse_obj_format(const char *str);
obj_model parse_obj_format(const std::string &str);
}
}
}

#include "obj_parser.inl"
