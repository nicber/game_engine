#pragma once

#include "opengl/vertex_array_object.h"

namespace game_engine {
namespace opengl {
namespace detail {
template <typename T>
GLenum get_constant();
}

template <typename T, typename F, unsigned I>
void vertex_array_object::bind_buffer_to_attrib(std::shared_ptr<buffer<T>> buff, size_t stride, size_t offset, vertex_attr attr) {
  static_assert(I > 0 && I <= 4, "OpenGL doesn't accept larger than 4 vertex"
                                 "attributes or 0-sized ones");
  bind();
  glBindBuffer(GL_ARRAY_BUFFER, buff->buffer_id);

  auto type_constant = detail::get_constant<F>();

  auto real_offset = offset * sizeof(T);
  if (offset != 0) {
    real_offset += stride % offset;
  }

  glVertexAttribPointer(attr, I, type_constant, false,
                        stride, reinterpret_cast<GLvoid*>(real_offset));

  glEnableVertexAttribArray(attr);
  bound_buffers.emplace_back(std::move(buff));
}
}
}
