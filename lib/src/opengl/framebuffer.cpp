#include <cassert>
#include "opengl/framebuffer.h"

namespace game_engine {
namespace opengl {
static GLuint read_and_draw_bound = 0;
static GLuint draw_bound = 0;
static GLuint read_bound = 0;

framebuffer::framebuffer() {
  glGenFramebuffers(1, &framebuffer_id);
}

framebuffer::~framebuffer() {
  if (read_and_draw_bound == framebuffer_id) {
    read_and_draw_bound = 0;
  } else if (draw_bound == framebuffer_id) {
    draw_bound = 0;
  } else if (read_bound == framebuffer_id) {
    read_bound = 0;
  }
  glDeleteFramebuffers(1, &framebuffer_id);
}

void framebuffer::bind_to(framebuffer::render_target target) {
  glBindFramebuffer(static_cast<GLenum>(target), framebuffer_id);
  switch (target) {
    case render_target::read_and_draw:
      read_and_draw_bound = framebuffer_id;
      break;
    case render_target::draw:
      draw_bound = framebuffer_id;
      break;
    case render_target::read:
      read_bound = framebuffer_id;
      break;
    default:
      assert(false);
  }
}

bool framebuffer::get_bind(render_target &ret) {
  if (read_and_draw_bound == framebuffer_id) {
    ret = render_target::read_and_draw;
    return  true;
  } else if (draw_bound == framebuffer_id) {
    ret = render_target::draw;
    return true;
  } else if (read_bound == framebuffer_id) {
    ret = render_target::read;
    return true;
  }
  return false;
}
}
}
