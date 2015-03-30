#include "opengl/renderbuffer.h"

namespace game_engine {
namespace opengl {
renderbuffer::renderbuffer() {
  glGenRenderbuffers(1, &renderbuffer_id);
}

renderbuffer::renderbuffer(renderbuffer_format fmt,
                           size_t width,
                           size_t height)
{
  glGenRenderbuffers(1, &renderbuffer_id);
  allocate_new_storage(fmt, width, height);
}

void renderbuffer::allocate_new_storage(renderbuffer_format fmt,
                                        size_t width,
                                        size_t height)
{
  bind();
  auto format = static_cast<GLenum>(fmt);
  glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
}

void renderbuffer::bind() {
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_id);
}
}
}
