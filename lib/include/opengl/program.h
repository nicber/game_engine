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
namespace render {
class mesh;
}
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

class program {
public:
  program(const shader &s1,
               const shader &s2,
               std::initializer_list<shader> shaders,
               std::initializer_list<attrib_pair> attribs,
               std::initializer_list<frag_data_loc> frag_data);

  program(program&& other);
  program &operator=(program&& other);

  ~program();

  bool has_vertex_attr(const std::string &name) const;
  vertex_attr &get_vertex_attr(const std::string &name) const;

  bool has_uniform(const std::string &name) const;
  uniform &get_uniform(const std::string &name) const;

  bool has_frag_loc(const std::string &name) const;
  frag_loc &get_frag_loc(const std::string &name) const;

  void bind() const;
private:
  program();

  template <typename T>
  using unor_map = std::unordered_map<std::string, T>;

  template <typename T>
  using unor_map_i = typename unor_map<T>::iterator;

  unor_map_i<uniform> find_uniform(const std::string &name) const;
  unor_map_i<vertex_attr> find_vertex_attr(const std::string &name) const;
  unor_map_i<frag_loc> find_frag_loc(const std::string &name) const;

private:
  friend class render::mesh;
  friend void swap(program &lhs, program &rhs);
  GLuint program_id = 0;
  mutable unor_map<uniform> uniforms;
  mutable unor_map<vertex_attr> vertex_attrs;
  mutable unor_map<frag_loc> frag_locs;
};
}
}
