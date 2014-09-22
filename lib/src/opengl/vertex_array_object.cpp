#include "opengl/vertex_array_object.h"

namespace game_engine {
namespace opengl {
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
