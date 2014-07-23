#pragma once

#include <initializer_list>
#include "opengl/frag_loc.h"
#include "opengl/shader.h"
#include "opengl/uniform.h"
#include "opengl/vertex_attr.h"
#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace game_engine {
namespace opengl {
using attrib_pair = std::tuple<std::string, GLuint>;
using frag_data_loc = std::tuple<std::string, GLuint>;

class program_construction_error : public std::runtime_error {
public:
  program_construction_error(std::string error,
                             const shader &s1,
                             const shader &s2,
                             std::initializer_list<shader> shaders,
                             std::initializer_list<attrib_pair> attribs,
                             std::initializer_list<frag_data_loc> frag_data);
};

class program_data {
public:
  program_data(const shader &s1,
               const shader &s2,
               std::initializer_list<shader> shaders,
               std::initializer_list<attrib_pair> attribs,
               std::initializer_list<frag_data_loc> frag_data);

private:
  GLuint program_id; 
  std::unordered_map<std::string, uniform> uniforms;
  std::unordered_map<std::string, vertex_attr> vertex_attrs;
  std::unordered_map<std::string, frag_loc> frag_locs;
};
}
}
