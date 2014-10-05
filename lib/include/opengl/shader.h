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


class shader {
public:
  shader(shader_type s_type, const std::string &source,
         std::initializer_list<std::string> deps);

  shader(shader&& other);
  shader& operator=(shader&& other);
  ~shader();

private:
  shader();

private:
  shader_type type;
  GLuint shader_id = 0;

  friend class program;
  friend void swap(shader &lhs, shader &rhs);
};
}
}
