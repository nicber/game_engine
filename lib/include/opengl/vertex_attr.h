#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class vertex_attr {
public:
  vertex_attr(GLint loc);

private:
  friend class vertex_array_object;
  
  GLint attr_bind_loc;
};
}
}
