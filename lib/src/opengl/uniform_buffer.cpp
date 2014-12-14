
#include "opengl/program.h"
#include "opengl/uniform_buffer.h"

namespace game_engine {
namespace opengl {
uniform_buffer::uniform_buffer(const program &prog, std::string block_name,
                               buf_freq_access freq_acc, buf_kind_access kind_acc):
  buffer<unsigned char>(construct_buffer_and_variables(prog, block_name, freq_acc, kind_acc)),
  data(std::make_shared<uniform_buffer_data>())
{
  data->uniform_block_name = std::move(block_name);
}

buffer<unsigned char>::iterator uniform_buffer::begin(const std::string &name) {
  auto it = data->variables.find(name);
  if (it == data->variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->second.block_offset;
}

buffer<unsigned char>::const_iterator uniform_buffer::begin(const std::string &name) const {
  auto it = data->variables.find(name);
  if (it == data->variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->second.block_offset;
}

buffer<unsigned char>::const_iterator uniform_buffer::cbegin(const std::string &name) const {
  return begin(name);
}

void uniform_buffer::bind_to(uniform_block_binding_handle handle) {
  handle->bound_buffer_data = data;
  glBindBufferBase(GL_UNIFORM_BUFFER, handle->id, get_buffer_id());
}

buffer<unsigned char> uniform_buffer::construct_buffer_and_variables(
                                      const program &prog, const std::string &name,
                                      buf_freq_access freq_acc,
                                      buf_kind_access kind_acc)
{
  GLint uniform_block_index = -1;
  GLint block_size;
  auto unif_iter = prog.get_uniforms();

  for (auto it = unif_iter.begin; it != unif_iter.end; ++it) {
    if (uniform_block_index != -1 && it->second.block_index == uniform_block_index) {
      data->variables.emplace(it->second.name, it->second);
      assert(block_size == it->second.block_size);
      continue;
    }

    if (it->second.block_name == name) {
      uniform_block_index = it->second.block_index;
      block_size = it->second.block_size;
    }
  }

  if (uniform_block_index == -1) {
    throw std::runtime_error("no uniform block with name: " + name);
  }

  return {(size_t)block_size, freq_acc, kind_acc};
}
}
}

