#include "opengl/vertex_array_object.h"

namespace game_engine {
namespace opengl {
namespace detail {
#define GET_CONSTANT(type, constant) \
template <> \
GLenum get_constant<type>() { \
  return constant; \
}

GET_CONSTANT(float, GL_FLOAT);

#undef GET_CONSTANT
}
vertex_array_object::vertex_array_object() {
  glGenVertexArrays(1, &vao_id);
}

vertex_array_object::~vertex_array_object() {
  glDeleteVertexArrays(1, &vao_id);
}

void vertex_array_object::bind() const {
  static GLuint currently_bound_vao = 0;
  if (currently_bound_vao != vao_id) {
    glBindVertexArray(vao_id);
  }
}
}
}
