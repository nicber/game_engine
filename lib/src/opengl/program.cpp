#include <cassert>
#include <opengl/program.h>
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

program::program(const shader &s1,
                           const shader &s2,
                           std::initializer_list<shader> shaders,
                           std::initializer_list<attrib_pair> attribs,
                           std::initializer_list<frag_data_loc> frag_data)
  {
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

program::~program() {
  glDeleteProgram(program_id);
}

program::program() {
}

bool program::has_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it == vertex_attrs.end();
}

vertex_attr &program::get_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it->second;
}

bool program::has_uniform(const std::string &name) const {
  auto it = find_uniform(name);
  return it == uniforms.end();
}

uniform &program::get_uniform(const std::string &name) const {
  auto it = find_uniform(name);
  return it->second;
}

bool program::has_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it == frag_locs.end();
}

frag_loc &program::get_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it->second;
}

void program::bind() const {
  static GLuint currently_bound_program = 0;
  if (program_id != currently_bound_program) {
    glUseProgram(program_id);
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

prg_unor_map_i<uniform> program::find_uniform(const std::string &name) const{
  return find_generic(program_id, name, uniforms, glGetUniformLocation);
}

prg_unor_map_i<vertex_attr> program::find_vertex_attr(const std::string &name) const {
  return find_generic(program_id, name, vertex_attrs, glGetAttribLocation);
}

prg_unor_map_i<frag_loc> program::find_frag_loc(const std::string &name) const {
  return find_generic(program_id, name, frag_locs, glGetFragDataLocation);
}
}
}
