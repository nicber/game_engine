#pragma once

#include <GL/glew.h>
#include <memory>
#include <string>
#include <vector>

namespace game_engine {
namespace opengl {
/** \brief Class that represents a uniform */
struct uniform {
  std::string name;
  std::string block_name;
  GLint block_index;
  GLint block_size;
  GLint block_offset;
  GLint array_size;
  GLint block_array_stride;
  GLint block_matrix_stride;
  bool matrix_row_major;
  GLenum type;
};

class uniform_setter : public uniform {
public:
  struct type_mismatch : std::invalid_argument {
    using std::invalid_argument::invalid_argument;
  };

public:
  uniform_setter(uniform u_, GLint loc_, GLuint prog_id_);
  uniform_setter();

  /** \brief Sets a uniform to t.
   * \throws type_mismatch if T is not compatible with the uniform's type.
   */
  template <typename T>
  void set(const T &t);

  /** \brief Sets an array of uniforms to the content of [begin; begin + count].
   */
  template <typename It>
  void set(GLsizei count, It begin);

  /** \brief Sets an array of uniforms to the content of [begin; end).
   */
  template <typename It>
  void set(It begin, It end);
private:
  GLint location;
  GLuint prog_id;
};

bool operator==(const uniform &lhs, const uniform &rhs);
bool operator!=(const uniform &lhs, const uniform &rhs);

/** \brief Utility function for querying information about a program's uniforms all at once.
 */
std::vector<uniform_setter> get_uniforms_of_program(GLuint prog_id);

struct uniform_buffer_data;
class uniform_buffer;

struct uniform_block_binding;
using uniform_block_binding_handle = std::shared_ptr<uniform_block_binding>;

struct uniform_block_binding {
  uniform_block_binding(GLint id_, std::string name_);

  const std::string &get_name() const;
  GLint get_id() const;
  std::shared_ptr<uniform_buffer_data> get_bound_buffer_data() const;

private:
  // friendship needed to allow uniform_buffer::bind_to to set bound_buffer_data.
  friend class uniform_buffer;

  // friendship needed to set and reset name.
  friend uniform_block_binding_handle get_free_uniform_block_binding(std::string name);
  GLint id;
  std::string name;
  std::weak_ptr<uniform_buffer_data> bound_buffer_data;
};


/** \brief Returns a uniform block binding that's free or the one that has previously been
 * assigned the name 'name'.
 * \throws std::runtime_error if there isn't a free binding to return and there isn't a binding
 * already bound to the name 'name'.
 */
uniform_block_binding_handle get_free_uniform_block_binding(std::string name);
}
}

#include "uniform.inl"
