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
  ~framebuffer();

  void bind_to(render_target target);

private:
  /** \brief Returns true and stores the render_target this framebuffer is
   * currently bound to. If there is no such target, it returns false and
   * does not touch the contents of the passed reference.
   */
  bool get_bind(render_target &ret);
private:
  GLuint framebuffer_id;
};
}
}
