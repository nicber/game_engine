#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
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

  using handles_vector = std::vector<std::pair<std::string,uniform_block_binding_handle>>;
private:
  /** \brief Useless constructor that needs to be here so that program can
   * construct a program_uniform_block_binding_manager during its construction.
   */
  program_uniform_block_binding_manager(program *ptr);
  friend class program;

private:
  friend void swap(program&, program&);
  handles_vector handles;
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
  using uniform_cont = boost::multi_index_container<
                         uniform,
                         boost::multi_index::indexed_by<
                           boost::multi_index::ordered_non_unique<
                             boost::multi_index::member<uniform, std::string, &uniform::block_name>
                           >,
                           boost::multi_index::ordered_non_unique<  
                             boost::multi_index::member<uniform, std::string, &uniform::name>
                           >
                         >
                       >;

  program(const shader &s1,
               const shader &s2,
               std::initializer_list<shader> shaders,
               std::initializer_list<attrib_pair> attribs,
               std::initializer_list<frag_data_loc> frag_data);

  program(program&& other);
  program &operator=(program&& other);

  ~program();

  bool has_vertex_attr(const std::string &name) const;
  const vertex_attr &get_vertex_attr(const std::string &name) const;

  bool has_uniform(const std::string &name) const;
  const uniform &get_uniform(const std::string &name) const;

  const uniform_cont &get_uniforms() const;

  bool has_frag_loc(const std::string &name) const;
  const frag_loc &get_frag_loc(const std::string &name) const;

  void bind() const;
private:
  program();

  template <typename T>
  using unor_map = std::unordered_map<std::string, T>;

  template <typename T>
  using unor_map_i = typename unor_map<T>::iterator;

  uniform_cont::nth_index<1>::type::const_iterator find_uniform(const std::string &name) const;
  unor_map_i<vertex_attr> find_vertex_attr(const std::string &name) const;
  unor_map_i<frag_loc> find_frag_loc(const std::string &name) const;

  /** \brief Returns the object that manages this program's uniform block
   * bindings.
   */
  program_uniform_block_binding_manager& ubb_manager();
  const program_uniform_block_binding_manager& ubb_manager() const;

  void setup_uniforms_if_nec() const;
private:
  friend class render::mesh;
  friend class program_uniform_block_binding_manager;
  friend void swap(program &lhs, program &rhs);
  GLuint program_id = 0;
  mutable uniform_cont uniforms;
  mutable bool uniforms_already_queried = false;
  mutable unor_map<vertex_attr> vertex_attrs;
  mutable unor_map<frag_loc> frag_locs;
  program_uniform_block_binding_manager bind_manager{this};
};
}
}
