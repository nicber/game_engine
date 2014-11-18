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
class program;

class program_uniform_block_binding_manager {
public:
  void add_binding(const std::string &block_name, const std::string &binding_name);
  void remove_binding_by_block_name(const std::string &block_name);
  void remove_binding_by_binding_name(const std::string &binding_name);

  /** \brief Checks if the buffer bound to every uniform block binding is
   * compatible with the program's uniform block bound to that uniform block
   * binding.
   */
  bool check_compatibility() const;

  /** \brief Checks if the buffer bound to uniform block binding
   * 'binding_name' is compatible with the program's uniform block bound to
   * that uniform block binding.
   */
  bool check_compatibility_binding(const std::string &binding_name) const;
private:
    /** \brief Useless constructor that needs to be here so that program can
     * construct a program_uniform_block_binding_manager during its construction.
     */
    program_uniform_block_binding_manager(program *ptr);
    friend class program;
private:
  friend void swap(program&, program&);
  std::vector<uniform_block_binding_handle> handles;
  program *prog_ptr = nullptr;
};

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

  struct const_uniform_iter {
    std::unordered_map<std::string, uniform>::const_iterator begin, end;
  };

  const_uniform_iter get_uniforms() const;

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

  /** \brief Returns the object that manages this program's uniform block
   * bindings.
   */
  program_uniform_block_binding_manager& ubb_manager();
  const program_uniform_block_binding_manager& ubb_manager() const;
private:
  friend class render::mesh;
  friend class program_uniform_block_binding_manager;
  friend void swap(program &lhs, program &rhs);
  GLuint program_id = 0;
  mutable unor_map<uniform> uniforms;
  mutable bool uniforms_already_queried = false;
  mutable unor_map<vertex_attr> vertex_attrs;
  mutable unor_map<frag_loc> frag_locs;
  program_uniform_block_binding_manager bind_manager{this};
};
}
}
