#include <GL/glew.h>

namespace game_engine {
namespace opengl {
enum class renderbuffer_format : GLenum {
  rgba4 = GL_RGBA4,
  rgb565 = GL_RGB565,
  rgb5_a1 = GL_RGB5_A1,
  depth_component16 = GL_DEPTH_COMPONENT16,
  stencil_index8 = GL_STENCIL_INDEX8
};

/** \brief Class that represents a renderbuffer. */
class renderbuffer {
public:
  /** \brief Creates a renderbuffer with no storage attached.
   * Use allocate_new_storage() to allocate storage.
   */
  renderbuffer();

  /** \brief Creates a render buffer with storage created according to the
   * passed parameters.
   * Equivalent to constructing an empty renderbuffer and then calling
   * allocate_new_storage().
   */
  renderbuffer(renderbuffer_format fmt, size_t width, size_t height);

  ~renderbuffer();

  /** \brief Allocates new storage for the renderbuffer deallocating any
   * existing one.
   */
  void allocate_new_storage(renderbuffer_format fmt, size_t width, size_t height);

private:
  void bind();
private:
  friend class framebuffer;
  GLuint renderbuffer_id;
};
}
}
