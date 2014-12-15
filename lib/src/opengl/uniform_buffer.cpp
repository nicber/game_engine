
#include "opengl/program.h"
#include "opengl/uniform_buffer.h"

namespace game_engine {
namespace opengl {
bool uniform_buff_variable_sort_func(const uniform &lhs, const uniform &rhs) {
  return lhs.name < rhs.name;
}

bool uniform_buff_variable_sort_with_name_func(const uniform &lhs, const std::string &rhs) {
  return lhs.name < rhs;
}

uniform_buffer::uniform_buffer(const program &prog, std::string block_name,
                               buf_freq_access freq_acc, buf_kind_access kind_acc):
  buffer<unsigned char>(construct_buffer_and_variables(prog, block_name, freq_acc, kind_acc))
{
  assert(data->variables.size() && "there can't be a uniform block with 0 variables");
  std::sort(data->variables.begin(), data->variables.end(), uniform_buff_variable_sort_func);
}

buffer<unsigned char>::iterator uniform_buffer::begin(const std::string &name) {
  auto it = std::lower_bound(data->variables.cbegin(), data->variables.cend(), name,
                             uniform_buff_variable_sort_with_name_func);
  if (it == data->variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->block_offset;
}

buffer<unsigned char>::const_iterator uniform_buffer::begin(const std::string &name) const {
  auto it = std::lower_bound(data->variables.cbegin(), data->variables.cend(), name,
                             uniform_buff_variable_sort_with_name_func);
  if (it == data->variables.end()) {
    throw std::runtime_error("no variable named " + name + " in this block");
  }

  return buffer<unsigned char>::begin() + it->block_offset;
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
  const auto &unifs = prog.get_uniforms();
  data = std::make_shared<uniform_buffer_data>();

  for (auto it = unifs.get<0>().cbegin(); it != unifs.get<0>().cend(); ++it) {
    if (uniform_block_index != -1 && it->block_index == uniform_block_index) {
      data->variables.emplace_back(*it);
      assert(block_size == it->block_size);
      continue;
    }

    if (it->block_name == name) {
      uniform_block_index = it->block_index;
      block_size = it->block_size;
    }
  }

  if (uniform_block_index == -1) {
    throw std::runtime_error("no uniform block with name: " + name);
  }

  return {(size_t)block_size, freq_acc, kind_acc};
}
}
}

