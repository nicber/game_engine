#include "opengl/framebuffer.h"
#include "opengl/output_location.h"
#include <boost/variant/variant.hpp>
#include <boost/variant/apply_visitor.hpp>

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
  auto it = find_equal_dest(dest);

  if (it == connections.cend()) {
    return boost::none;
  } else {
    return it->first;
  }
}

bool output_location_set::remove_connection(GLuint loc)
{
  if (connections.erase(loc)) {
    vector_uptodate = false;
    return true;
  } else {
    return false;
  }
}

bool output_location_set::remove_connection(connection_dest dest)
{
  auto it = find_equal_dest(dest);
  if (it != connections.end()) {
    connections.erase(it);
    return true;
  } else {
    return false;
  }
}

void output_location_set::apply_to(framebuffer &fb) const
{
  update_vector_if_nec();
  if (attachments_in_order.empty()) {
    return;
  }
  fb.bind_to(framebuffer::render_target::draw);
  glDrawBuffers(attachments_in_order.size(), attachments_in_order.data());
}

void output_location_set::update_vector_if_nec() const
{
  if (vector_uptodate) {
    return;
  }

  vector_uptodate = true;

  if (connections.empty()) {
    attachments_in_order.clear();
  } else {
    auto last = connections.rbegin()->first;
    attachments_in_order.reserve(last + 1);
    attachments_in_order.clear();
    std::fill_n(std::back_inserter(attachments_in_order),
                last + 1, GL_NONE);

    for (auto & pair : connections) {
      auto target = boost::apply_visitor(get_gl_constant(), pair.second);
      attachments_in_order[pair.first] = target;
    }
  }
}

output_location_set::map_loc_dest::const_iterator
output_location_set::find_equal_dest(connection_dest dest) const
{
  auto it = std::find_if(connections.cbegin(), connections.cend(),
            [&dest](auto &pair) {
              return pair.second == dest;
            });
  return it;
}
}
}
