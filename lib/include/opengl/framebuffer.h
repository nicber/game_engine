#pragma once
#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class framebuffer {
public:
  enum class render_target : GLenum {
    read = GL_DRAW_FRAMEBUFFER,
    draw = GL_READ_FRAMEBUFFER,
    read_and_draw = GL_FRAMEBUFFER
  };

public:
  framebuffer();

  void bind_to(render_target target);

private:
  GLuint framebuffer_id;
};
}
}
