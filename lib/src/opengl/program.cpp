#include <algorithm>
#include <cassert>
#include "opengl/program.h"
#include "opengl/uniform_buffer.h"
#include <sstream>
#include <unordered_set>

namespace game_engine {
namespace opengl {
program_construction_error::program_construction_error(std::string error,
                            const shader&,
                            const shader&,
                            std::initializer_list<shader>,
                            std::initializer_list<attrib_pair>,
                            std::initializer_list<frag_data_loc>)
 : std::runtime_error(std::move(error))
{}

program_uniform_block_binding_manager::program_uniform_block_binding_manager(program *ptr)
 : prog_ptr(ptr)
{}

static program::uniform_cont::nth_index<0>::type::const_iterator find_block_by_name(const program &prog, const std::string &name) {
  prog.bind();
  const auto &unifs = prog.get_uniforms();
  auto it = unifs.get<0>().lower_bound(name);
  if (it == unifs.get<0>().cend()) {
    throw std::runtime_error(std::string("no uniform block named ") + name);
  }
  return it;
}

static auto find_binding_by_name(const program_uniform_block_binding_manager::handles_vector &hv,
                                 const std::string &binding_name) {
  auto it = std::find_if(hv.begin(), hv.end(),
      [&binding_name](decltype(hv[0])& handle) {
        return handle.second->get_name() == binding_name;
      });

  if (it == hv.end()) {
    throw std::runtime_error("no uniform block binding named " + binding_name);
  }
  return it;
}

void program_uniform_block_binding_manager::add_binding(const std::string &block_name,
                                                        const std::string &binding_name) {
  auto it = find_block_by_name(*prog_ptr, block_name);
  auto binding = get_free_uniform_block_binding(binding_name);
  glUniformBlockBinding(prog_ptr->program_id, it->block_index, binding->get_id());
  handles.emplace_back(block_name, std::move(binding));
}

void program_uniform_block_binding_manager::remove_binding_by_binding_name(const std::string &binding_name) {
  auto it = find_binding_by_name(handles, binding_name);
  handles.erase(it);
}

bool program_uniform_block_binding_manager::check_compatibility() const {
  for (auto& handle : handles) {
    if (!check_compatibility_binding(handle.second->get_name())) {
      return false;
    }
  }
  return true;
}

bool program_uniform_block_binding_manager::check_compatibility_binding (const std::string &binding_name) const {
  handles_vector::const_iterator bind_it;
  program::uniform_cont::nth_index<0>::type::const_iterator block_it;

  try {
    bind_it = find_binding_by_name(handles, binding_name);
    block_it = find_block_by_name(*prog_ptr, bind_it->first);
  } catch (std::runtime_error &e) {
    return false;
  }

  auto bound_buffer_info = bind_it->second->get_bound_buffer_data();
  if (!bound_buffer_info) {
    return false;
  }

  const auto &buffer_variables = bound_buffer_info->variables;

  // the name the block the binding block is bound to should be
  // the same as the block name the buffer is based on.
  if (bind_it->first != buffer_variables[0].block_name) {
    return false;
  }

  do {
    auto same_name_it = std::lower_bound(buffer_variables.cbegin(),
                                         buffer_variables.cend(),
                                         *block_it,
                                         uniform_buff_variable_sort_func);
    if (same_name_it == buffer_variables.cend()) {
      return false;
    }
    // lets compare the rest of the variable, size, type, etc.
    if (*same_name_it != *block_it) {
      return false;
    }
  } while (++block_it != prog_ptr->uniforms.get<0>().cend() &&
           block_it->block_name == bind_it->first);
  return true;
}

program::program(const shader &s1,
                           const shader &s2,
                           std::initializer_list<shader> shaders,
                           std::initializer_list<attrib_pair> attribs,
                           std::initializer_list<frag_data_loc> frag_data) {
  size_t vertex_count = 0;
  size_t fragment_count = 0;

  auto test = [&] (const shader &s) {
    if (s.type == shader_type::vertex) {
      ++vertex_count;
    } else if (s.type == shader_type::fragment) {
      ++fragment_count;
    } else {
      assert(false && "unknown shader type");
    }
  };

  test(s1);
  test(s2);
  for (auto& s : shaders) {
    test(s);
  }


  if (vertex_count == 0) {
    throw program_construction_error("no vertex shaders", s1, s2, shaders,
                                     attribs, frag_data);
  } else if (fragment_count == 0) {
    throw program_construction_error("no fragment shaders", s1, s2, shaders,
                                     attribs, frag_data);
  }

  program_id = glCreateProgram();

  glAttachShader(program_id, s1.shader_id);
  glAttachShader(program_id, s2.shader_id);
  for (auto& s : shaders) {
    glAttachShader(program_id, s.shader_id);
  }

  for (auto& a_p : attribs) {
    glBindAttribLocation(program_id,
                         std::get<1>(a_p),
                         std::get<0>(a_p).c_str());

    vertex_attrs.emplace(std::move(std::get<0>(a_p)),
                        vertex_attr(std::get<1>(a_p)));
  }

  for (auto& f_d : frag_data) {
    glBindFragDataLocation(program_id,
                           std::get<1>(f_d),
                           std::get<0>(f_d).c_str());

    frag_locs.emplace(std::move(std::get<0>(f_d)),
                      frag_loc(std::get<1>(f_d)));
  }

  glLinkProgram(program_id);

  GLint compiled;
  glGetProgramiv(program_id, GL_LINK_STATUS, &compiled);
  if (!compiled) {
    GLint log_len;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_len);

    std::vector<char> log(log_len);
    glGetProgramInfoLog(program_id, log_len, nullptr, log.data());

    glDeleteProgram(program_id);

    throw program_construction_error(std::string(log.data()), s1, s2, shaders,
                                     attribs, frag_data);
  }

  glDetachShader(program_id, s1.shader_id);
  glDetachShader(program_id, s2.shader_id);
  for (auto& s : shaders) {
    glDetachShader(program_id, s.shader_id);
  }
}

program::program(program&& other) {
  using std::swap;
  program temp;
  swap(*this, temp);
  swap(*this, other);
}

program &program::operator=(program&& other) {
  using std::swap;
  program temp;
  swap(*this, temp);
  swap(*this, other);

  return *this;
}

void swap(program &lhs, program &rhs) {
  using std::swap;
  swap(lhs.program_id, rhs.program_id);
  swap(lhs.uniforms, rhs.uniforms);
  swap(lhs.uniforms_already_queried, rhs.uniforms_already_queried);
  swap(lhs.vertex_attrs, rhs.vertex_attrs);
  swap(lhs.frag_locs, rhs.frag_locs);
  swap(lhs.bind_manager, rhs.bind_manager);
  lhs.bind_manager.prog_ptr = &lhs;
  rhs.bind_manager.prog_ptr = &rhs;
}

program::~program() {
  glDeleteProgram(program_id);
}

program::program() {
}

bool program::has_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it != vertex_attrs.end();
}

const vertex_attr &program::get_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it->second;
}

bool program::has_uniform(const std::string &name) const {
  auto it = find_uniform(name);
  return it != uniforms.get<1>().end();
}

uniform_setter program::get_uniform(const std::string &name) const {
  auto it = uniforms.get<1>().find(name);
  return *it;
}

const program::uniform_cont &program::get_uniforms() const {
  setup_uniforms_if_nec();
  return uniforms;
}

bool program::has_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it != frag_locs.end();
}

const frag_loc &program::get_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it->second;
}

void program::bind() const {
  bind_program(program_id);
}

void bind_program(GLuint prog_id) {
  static GLuint currently_bound_program = 0;
  if (prog_id != currently_bound_program) {
    glUseProgram(prog_id);
    currently_bound_program = prog_id;
  }
}

template <typename C, typename F>
typename C::iterator find_generic(GLuint program_id, const std::string &name, C &cont, F func) {
  auto it = cont.find(name);

  if (it != cont.end()) {
    return it;
  } else {
    auto loc = func(program_id, name.c_str());
	if (loc == -1) {
      return cont.end();
	} else {
      auto ret = cont.emplace(name, typename C::mapped_type(loc));
	  assert(ret.second && "cont was supposed not to have name as a key");
	  return ret.first;
	}
  }
}

template <typename T>
using prg_unor_map_i = typename program::unor_map_i<T>;

prg_unor_map_i<vertex_attr> program::find_vertex_attr(const std::string &name) const {
  return find_generic(program_id, name, vertex_attrs, glGetAttribLocation);
}

prg_unor_map_i<frag_loc> program::find_frag_loc(const std::string &name) const {
  return find_generic(program_id, name, frag_locs, glGetFragDataLocation);
}

program::uniform_cont::nth_index<1>::type::const_iterator program::find_uniform(const std::string &name) const {
  setup_uniforms_if_nec();
  return uniforms.get<1>().find(name);
}

program_uniform_block_binding_manager &program::ubb_manager() {
  assert (bind_manager.prog_ptr == this);
  return bind_manager;
}

const program_uniform_block_binding_manager &program::ubb_manager() const {
  assert (bind_manager.prog_ptr == this);
  return bind_manager;
}

void program::setup_uniforms_if_nec() const {
  if (!uniforms_already_queried) {
    uniforms_already_queried = true;
    auto unis = get_uniforms_of_program(program_id);
    for (auto &uni : unis) {
      uniforms.insert(std::move(uni));
    }
  }
}
}
}
