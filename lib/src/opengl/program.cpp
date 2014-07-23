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
  {/*
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
  }*/
}
}
}
