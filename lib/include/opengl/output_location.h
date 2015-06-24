#pragma once

#include <boost/optional.hpp>
#include <boost/variant/variant_fwd.hpp>
#include <GL/glew.h>
#include <map>

namespace game_engine {
namespace opengl {
struct attachment;
class framebuffer;
/** \brief This enum is used for specifying where shader outputs are to be
 * written when drawing.
 */
enum class positional_output : GLenum
{
  none = GL_NONE,
  front_left = GL_FRONT_LEFT,
  front_right = GL_FRONT_RIGHT,
  back_left = GL_BACK_LEFT,
  back_right = GL_BACK_RIGHT,
};

using connection_dest = boost::variant<attachment, positional_output>;

/** \brief This class acts as a container of pairs of locations of shader
 * outputs and framebuffer attachments.
 * It merely remembers the desired connections but it doesn't apply the
 * necessary changes to the global state until the apply_to() member function
 * is called.
 */
class output_location_set {
public:
  /** \brief Setup a connection.
   * \param dest The framebuffer attachment where to write the program output.
   * \param location The location where the program writes its output.
   */
  void connect(connection_dest dest, GLuint location);

  /** \brief Gets what attachment a specific location is to be bound to.
   * \param loc The location.
   */
  connection_dest get_connection(GLuint loc) const;

  /** \brief Gets what location the data written to a specific attachment comes
   * from.
   * \param dest The attachment to query about.
   */
  boost::optional<GLuint> get_connection(connection_dest dest) const;

  /** \brief Applies necessary changes to the global state to make sure that
   * program outputs will be written to the appropriate attachments of the
   * framebuffer specified by the argument.
   * \param fb Framebuffer to make the destination for outputs according to the
   * connections specified by this object.
   */
  void apply_to(framebuffer &fb) const;

private:
  void update_vector_if_nec() const;

private:
  mutable std::vector<GLenum> attachments_in_order;
  std::map<GLuint, connection_dest> connections;
  mutable bool vector_uptodate = false;
};
}
}
