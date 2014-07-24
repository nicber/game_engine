#include <cassert>
#include <opengl/program.h>
#include <sstream>
#include <unordered_set>

namespace game_engine {
namespace opengl {
program_data::program_data(const shader &s1,
                           const shader &s2,
                           std::initializer_list<shader> shaders,
                           std::initializer_list<attrib_pair> attribs,
                           std::initializer_list<frag_data_loc> frag_data)
  {
  size_t vertex_count = 0;
  size_t fragment_count = 0;

  auto test = [&] (const shader &s) {
    if (s.get().type == shader_type::vertex) {
      ++vertex_count;
    } else if (s.get().type == shader_type::fragment) {
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

  glAttachShader(program_id, s1.get().shader_id);
  glAttachShader(program_id, s2.get().shader_id);
  for (auto& s : shaders) {
    glAttachShader(program_id, s.get().shader_id);
  }

  for (auto& a_p : attribs) {
    glBindAttribLocation(program_id,
                         std::get<1>(a_p),
                         std::get<0>(a_p).c_str());

    vertex_attrs.emplace(std::move(std::get<0>(a_p)),
                        std::get<1>(a_p));
  }

  for (auto& f_d : frag_data) {
    glBindFragDataLocation(program_id,
                           std::get<1>(f_d),
                           std::get<0>(f_d).c_str());

    frag_locs.emplace(std::move(std::get<0>(f_d)),
                      std::get<1>(f_d));
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

  glDetachShader(program_id, s1.get().shader_id);
  glDetachShader(program_id, s2.get().shader_id);
  for (auto& s : shaders) {
    glDetachShader(program_id, s.get().shader_id);
  }
}

program_data::program_data(program_data&& other) {
  using std::swap;
  program_data temp;
  swap(*this, temp);
  swap(*this, other);
}

program_data &program_data::operator=(program_data&& other) {
  using std::swap;
  program_data temp;
  swap(*this, temp);
  swap(*this, other);

  return *this;
}

program_data::~program_data() {
  glDeleteProgram(program_id);
}

program_data::program_data() {
}

bool program_data::has_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it == vertex_attrs.end();
}

vertex_attr &program_data::get_vertex_attr(const std::string &name) const {
  auto it = find_vertex_attr(name);
  return it->second;
}

bool program_data::has_uniform(const std::string &name) const {
  auto it = find_uniform(name);
  return it == uniforms.end();
}

uniform &program_data::get_uniform(const std::string &name) const {
  auto it = find_uniform(name);
  return it->second;
}

bool program_data::has_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it == frag_locs.end();
}

frag_loc &program_data::get_frag_loc(const std::string &name) const {
  auto it = find_frag_loc(name);
  return it->second;
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
      auto ret = cont.emplace(name, loc);
	  assert(ret.second && "cont was supposed not to have name as a key");
	  return ret.first;
	}
  }
}

template <typename T>
using prg_unor_map_i = typename program_data::unor_map_i<T>;

prg_unor_map_i<uniform> program_data::find_uniform(const std::string &name) const{
  return find_generic(program_id, name, uniforms, glGetUniformLocation);
}

prg_unor_map_i<vertex_attr> program_data::find_vertex_attr(const std::string &name) const {
  return find_generic(program_id, name, vertex_attrs, glGetAttribLocation);
}

prg_unor_map_i<frag_loc> program_data::find_frag_loc(const std::string &name) const {
  return find_generic(program_id, name, frag_locs, glGetFragDataLocation);
}
}
}
