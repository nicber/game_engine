#pragma once

#include <GL/glew.h>
#include <vector>

namespace game_engine {
namespace opengl {

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

std::vector<uniform> get_uniforms_of_program(GLuint prog_id);

struct uniform_block_binding {
  uniform_block_binding(GLint id_, std::string name_);

  GLint id;
  std::string name;
};

std::shared_ptr<const uniform_block_binding> get_free_uniform_block_binding(std::string name);
}
}
