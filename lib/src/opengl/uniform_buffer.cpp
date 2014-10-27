
#include "opengl/program.h"
#include "opengl/uniform_buffer.h"

namespace game_engine {
namespace opengl {
uniform_buffer::uniform_buffer(const program &prog, std::string block_name,
                               buf_freq_access freq_acc, buf_kind_access kind_acc):
  buffer<unsigned char>(construct_buffer_and_variables(prog, block_name, freq_acc, kind_acc)),
  uniform_block_name(std::move(block_name))
{
}

buffer<unsigned char>::iterator uniform_buffer::begin(const std::string &name) {
  auto it = variables.find(name);
  if (it == variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->second.block_offset;
}

buffer<unsigned char>::const_iterator uniform_buffer::begin(const std::string &name) const {
  auto it = variables.find(name);
  if (it == variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->second.block_offset;
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
    if (it->second.name == name) {
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

