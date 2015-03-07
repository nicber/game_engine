#include "opengl/framebuffer.h"

namespace game_engine {
namespace opengl {
framebuffer::framebuffer() {
  glGenFramebuffers(1, &framebuffer_id);
}

framebuffer::~framebuffer() {
  glDeleteFramebuffers(1, &framebuffer_id);
}

void framebuffer::bind_to(framebuffer::render_target target) {
  glBindFramebuffer(static_cast<GLenum>(target), framebuffer_id);
}
}
}
