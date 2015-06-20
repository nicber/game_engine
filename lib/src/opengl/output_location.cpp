#include "opengl/output_location.h"

namespace game_engine {
namespace opengl {
struct check_not_stencil_or_depth : public boost::static_visitor<bool> {
  bool operator()(attachment at) const
  {
    return at != attachment::stencil_attachment
        && at != attachment::depth_attachment
        && at != attachment::depth_stencil_attachment;
  }

  bool operator()(positional_output) const
  {
    return true;
  }
};

struct get_gl_constant : public boost::static_visitor<GLenum> {
  GLenum operator()(attachment at) const
  {
    auto constant = at.get_gl_constant();
    assert(constant != GL_STENCIL_ATTACHMENT);
    assert(constant != GL_DEPTH_ATTACHMENT);
    assert(constant != GL_DEPTH_STENCIL_ATTACHMENT);
    return constant;
  }

  GLenum operator()(positional_output pout) const
  {
    return static_cast<GLenum>(pout);
  }
};

void output_location_set::connect(connection_dest dest, GLuint location)
{
  if (!boost::apply_visitor(check_not_stencil_or_depth(), dest)) {
    throw std::invalid_argument("wrong type of attachment passed");
  }

  vector_uptodate = false;
  connections[location] = dest;
}

connection_dest output_location_set::get_connection(GLuint loc) const
{
  auto it = connections.find(loc);
  if (it == connections.end()) {
    return positional_output::none;
  } else {
    return it->second;
  }
}

boost::optional<GLuint> output_location_set::get_connection(connection_dest dest) const
{
  auto it = std::find_if(connections.cbegin(), connections.cend(),
            [dest](auto &pair) {
              return pair.second == dest;
            });

  if (it == connections.cend()) {
    return boost::none;
  } else {
    return it->first;
  }
}

void output_location_set::apply_to(framebuffer &fb) const
{
  update_vector_if_nec();
  fb.bind_to(framebuffer::render_target::draw);
  glDrawBuffers(attachments_in_order.size(), attachments_in_order.data());
}

void output_location_set::update_vector_if_nec() const
{
  if (vector_uptodate) {
    return;
  }

  vector_uptodate = true;
  attachments_in_order.reserve(connections.size());
  attachments_in_order.clear();
  std::fill_n(std::back_inserter(attachments_in_order),
              connections.size(), GL_NONE);

  for (auto & pair : connections) {
    auto target = boost::apply_visitor(get_gl_constant(), pair.second);
    attachments_in_order[pair.first] = target;
  }
}
}
}
