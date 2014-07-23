#pragma once

#include <boost/flyweight.hpp>
#include <GL/glew.h>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>
#include <SFML/OpenGL.hpp>

namespace game_engine {
namespace opengl {
class shader_error : public std::runtime_error {
public:
  shader_error(std::string error_log, std::string source,
               std::vector<std::string> deps);

private:
  std::string error_message;
  std::vector<std::string> sources;
};

enum class shader_type : GLenum {
  vertex = GL_VERTEX_SHADER,
  fragment = GL_FRAGMENT_SHADER
};


class shader_data {
public:
  shader_data(shader_type s_type, const std::string &source,
         std::initializer_list<std::string> deps);

  shader_data(shader_data&& other);
  shader_data& operator=(shader_data&& other);
  ~shader_data();
  
private:
  shader_data();

private:
  shader_type type;
  GLuint shader_id = 0;

  friend void swap(shader_data& lhs, shader_data& rhs);
  friend class program_data;
};

void swap(shader_data& lhs, shader_data& rhs);

using shader = boost::flyweight<shader_data>;
}
}
