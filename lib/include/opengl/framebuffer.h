#pragma once
#include <boost/functional/hash_fwd.hpp>
#include <GL/glew.h>
#include <memory>
#include "renderbuffer.h"
#include <unordered_map>

namespace game_engine {
namespace opengl {
struct attachment {
  static attachment color_attachmenti(size_t i);
  static attachment depth_attachment;
  static attachment stencil_attachment;
  static attachment depth_stencil_attachment;

  bool operator==(const attachment &rhs) const;
  bool operator!=(const attachment &rhs) const;

private:
  friend attachment init_att(GLenum constant);
  friend class framebuffer;
  friend size_t hash_value(const attachment &rnd_at);
  GLenum gl_constant;
};

size_t hash_value(const attachment &rnd_at);

/** \brief Class that represents a framebuffer. It provides several member
 * functions that abstract the OpenGL API.
 */
class framebuffer {
public:
  /** \brief This enum provides type safety when using framebuffer targets. */
  enum class render_target : GLenum {
    read = GL_DRAW_FRAMEBUFFER,
    draw = GL_READ_FRAMEBUFFER,
    read_and_draw = GL_FRAMEBUFFER
  };

public:
  framebuffer();
  ~framebuffer();

  /** \brief Binds the framebuffer to a certain render_target. */
  void bind_to(render_target target);

  /** \brief Attaches a passed renderbuffer to a specific attachment.
   * It takes a shared_ptr that is converted to a weak_ptr which is then
   * checked to see if no renderbuffer bound to an attachment of this
   * framebuffer has been destroyed. */
  void attach(attachment rnd_at,
              const std::shared_ptr<const renderbuffer> &ptr);

  /** \brief Attaches a passed renderbuffer to a specific attachment.
   * It does not take a shared_ptr to let the user manage renderbuffers as they
   * see fit. */
  void attach(attachment rnd_at,
              const renderbuffer& rbuffer);


private:
  /** \brief Returns true and stores the render_target this framebuffer is
   * currently bound to. If there is no such target, it returns false and
   * does not touch the contents of the passed reference. */
  bool get_bind(render_target &ret);

  void do_attach(attachment rnd_at,
                 const renderbuffer &rbuffer);

private:
  GLuint framebuffer_id;
  std::unordered_map<attachment,
                     std::weak_ptr<const renderbuffer>,
                     boost::hash<attachment>
                    > rbuff_attachments;
};
}
}
