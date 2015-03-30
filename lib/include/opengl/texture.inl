#pragma once

#include <cassert>
#include <iterator>
#include "texture.h"
#include <type_traits>
#include "uniform.h"
#include <util/iter_to_ptr.h>

namespace game_engine {
namespace opengl {
template <typename It>
void texture::upload(cube_map_side side, GLsizei size, GLuint level,
                     texture_internal_format i_f, texture_format fmt,
                     It begin, It end, data_type d_type)
{
  using T = std::remove_const_t<typename std::iterator_traits<It>::value_type>;
  GLenum data_type_enum;
  auto data_ptrs(util::trans_iter_to_ptr(begin, end));

  if (d_type != data_type::deduce) {
    data_type_enum = static_cast<GLenum>(d_type);
  } else {
    data_type_enum = gl_type<T>;
  }

  bind();

  glTexImage2D(static_cast<GLenum>(side),
               level,
               static_cast<GLenum>(i_f),
               size,
               size,
               0, // border has to be 0.
               static_cast<GLenum>(fmt),
               data_type_enum,
               data_ptrs.begin);
}

template <typename It>
void texture::upload(GLsizei width, GLsizei height, GLuint level,
                     texture_internal_format i_f, texture_format fmt,
                     It begin, It end, data_type d_type)
{
  using T = std::remove_const_t<typename std::iterator_traits<It>::value_type>;
  GLenum data_type_enum;
  auto data_ptrs(util::trans_iter_to_ptr(begin, end));

  if (d_type != data_type::deduce) {
    data_type_enum = static_cast<GLenum>(d_type);
  } else {
    data_type_enum = gl_type<T>;
  }

  assert(type == static_cast<GLenum>(texture_type::two_d));

  bind();

  glTexImage2D(static_cast<GLenum>(type),
               level,
               static_cast<GLenum>(i_f),
               width,
               height,
               0, // border has to be 0.
               static_cast<GLenum>(fmt),
               data_type_enum,
               data_ptrs.begin);

}
}
}
