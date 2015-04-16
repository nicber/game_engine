#include <boost/functional/hash.hpp>
#include <cassert>
#include "opengl/framebuffer.h"

namespace game_engine {
namespace opengl {
attachment init_att(GLenum constant) {
  attachment r;
  r.gl_constant = constant;
  return r;
}

attachment attachment::color_attachmenti(size_t i) {
  return init_att(i);
}

attachment attachment::depth_attachment =
  init_att(GL_DEPTH_ATTACHMENT);

attachment attachment::stencil_attachment =
  init_att(GL_STENCIL_ATTACHMENT);

attachment attachment::depth_stencil_attachment =
  init_att(GL_DEPTH_STENCIL_ATTACHMENT);

bool attachment::operator==(const attachment &rhs) const
{
  return gl_constant == rhs.gl_constant;
}

bool attachment::operator!=(const attachment &rhs) const
{
  return gl_constant != rhs.gl_constant;
}

size_t hash_value(const attachment &rnd_at)
{
  return boost::hash_value(rnd_at.gl_constant);
}

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

void framebuffer::attach(attachment rnd_at,
                         const std::shared_ptr<const renderbuffer> &ptr)
{
  if (rnd_at != attachment::depth_stencil_attachment) {
    rbuff_attachments[rnd_at] = ptr;
  } else {
    rbuff_attachments[attachment::depth_attachment] = ptr;
    rbuff_attachments[attachment::stencil_attachment] = ptr;
  }
  attach(rnd_at, *ptr);
}

void framebuffer::attach(attachment rnd_at,
                         const renderbuffer &rbuffer)
{
  auto remove_attachment = [&] (attachment att) {
    auto it = rbuff_attachments.find(att);
    if (it != rbuff_attachments.end() && it->second.lock().get() != &rbuffer) {
      rbuff_attachments.erase(it);
    }
  };

  if (rnd_at != attachment::depth_stencil_attachment) {
    remove_attachment(rnd_at);
  } else {
    remove_attachment(attachment::stencil_attachment);
    remove_attachment(attachment::depth_attachment);
  }

  do_attach(rnd_at, rbuffer);
}

void framebuffer::do_attach(attachment rnd_at,
                         const renderbuffer &rbuffer)
{
  render_target framebuffer_binding;
  if (!get_bind(framebuffer_binding)) {
    framebuffer_binding = render_target::read_and_draw;
    bind_to(framebuffer_binding);
  }

  glFramebufferRenderbuffer(static_cast<GLenum>(framebuffer_binding),
                            rnd_at.gl_constant,
                            GL_RENDERBUFFER,
                            rbuffer.renderbuffer_id);
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
