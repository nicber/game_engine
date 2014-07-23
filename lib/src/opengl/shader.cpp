#include "opengl/shader.h"

#include <algorithm>

namespace game_engine {
namespace opengl {
shader_error::shader_error(std::string error_log, std::string source,
                           std::vector<std::string> deps):
  std::runtime_error(std::move(error_log))
  {
  sources.reserve(1 + deps.size());
  sources.emplace_back(source);
  std::move(std::begin(deps), std::end(deps), std::back_inserter(sources));
}

shader_data::shader_data(shader_type s_type, const std::string &s_source,
               std::initializer_list<std::string> deps):
  type(s_type)
  {
  std::vector<const GLchar*> starts;
  starts.reserve(1 + deps.size());

  std::transform(std::begin(deps), std::end(deps), std::back_inserter(starts),
    [] (const std::string& str) { return str.c_str(); } );
  
  starts.emplace_back(s_source.c_str());

  shader_id = glCreateShader(static_cast<GLenum>(type));
  if (shader_id == 0) {
    throw std::runtime_error("error creating shader");      
  }

  glShaderSource(shader_id, 1 + deps.size(),
                 starts.data(),
                 nullptr);
  
  glCompileShader(shader_id);

  GLint compilation_res;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compilation_res);
  if(!compilation_res) {
    GLint log_len;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_len);
    std::vector<char> log(log_len);
    glGetShaderInfoLog(shader_id, log_len, nullptr, log.data());

    std::vector<std::string> sources;
    sources.reserve(deps.size());
    std::move(std::begin(deps), std::end(deps), std::back_inserter(sources));

    throw shader_error(std::string(log.data()), s_source,
                       sources);
  }
}

shader_data::shader_data(shader_data&& other) {
  using std::swap;
  swap(type, other.type);
  swap(shader_id, other.shader_id);
}

shader_data& shader_data::operator=(shader_data&& other) {
  using std::swap;
  shader_data temp;
  swap(*this, temp);
  swap(*this, other);

  return *this;
}

shader_data::~shader_data() {
  glDeleteShader(shader_id);
}
}
}
