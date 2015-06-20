#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
enum class texture_type : GLenum {
  two_d = GL_TEXTURE_2D,
  cube_map = GL_TEXTURE_CUBE_MAP
};

enum class texture_format {
  red = GL_RED,
  rg = GL_RG,
  rgb = GL_RGB,
  bgr = GL_BGR,
  rgba = GL_RGBA,
  bgra = GL_BGRA,
  red_integer = GL_RED_INTEGER,
  rg_integer = GL_RG_INTEGER,
  rgb_integer = GL_RGB_INTEGER,
  bgr_integer = GL_BGR_INTEGER,
  rgba_integer = GL_RGBA_INTEGER,
  bgra_integer = GL_BGRA_INTEGER,
  stencil_index = GL_STENCIL_INDEX,
  depth_component = GL_DEPTH_COMPONENT,
  depth_stencil = GL_DEPTH_STENCIL
};

enum class texture_internal_format {
  depth_component = GL_DEPTH_COMPONENT,
  depth_stencil = GL_DEPTH_STENCIL,
  red = GL_RED,
  rg = GL_RG,
  rgb = GL_RGB,
  rgba = GL_RGBA,
  r8 = GL_R8,
  r8_snorm = GL_R8_SNORM,
  r16 = GL_R16,
  r16_snorm = GL_R16_SNORM,
  rg8 = GL_RG8,
  rg8_snorm = GL_RG8_SNORM,
  rg16 = GL_RG16,
  rg16_snorm = GL_RG16_SNORM,
  r3_g3_b2 = GL_R3_G3_B2,
  rgb4 = GL_RGB4,
  rgb5 = GL_RGB5,
  rgb8 = GL_RGB8,
  rgb8_snorm = GL_RGB8_SNORM,
  rgb10 = GL_RGB10,
  rgb12 = GL_RGB12,
  rgb16_snorm = GL_RGB16_SNORM,
  rgba2 = GL_RGBA2,
  rgba4 = GL_RGBA4,
  rgb5_a1 = GL_RGB5_A1,
  rgba8 = GL_RGBA8,
  rgba8_snorm = GL_RGBA8_SNORM,
  rgb10_a2 = GL_RGB10_A2,
  rgb10_a2ui = GL_RGB10_A2UI,
  rgba12 = GL_RGBA12,
  rgba16 = GL_RGBA16,
  srgb8 = GL_SRGB8,
  srgb8_alpha8 = GL_SRGB8_ALPHA8,
  r16f = GL_R16F,
  rg16f = GL_RG16F,
  rgb16f = GL_RGB16F,
  rgba16f = GL_RGBA16F,
  r32f = GL_R32F,
  rg32f = GL_RG32F,
  rgb32f = GL_RGB32F,
  rgba32f = GL_RGBA32F,
  r11f_g11f_b10f = GL_R11F_G11F_B10F,
  rgb9_e5 = GL_RGB9_E5,
  r8i = GL_R8I,
  r8ui = GL_R8UI,
  r16i = GL_R16I,
  r16ui = GL_R16UI,
  r32i = GL_R32I,
  r32ui = GL_R32UI,
  rg8i = GL_RG8I,
  rg8ui = GL_RG8UI,
  rg16i = GL_RG16I,
  rg16ui = GL_RG16UI,
  rg32i = GL_RG32I,
  rg32ui = GL_RG32UI,
  rgb8i = GL_RGB8I,
  rgb8ui = GL_RGB8UI,
  rgb16i = GL_RGB16I,
  rgb16ui = GL_RGB16UI,
  rgb32i = GL_RGB32I,
  rgb32ui = GL_RGB32UI,
  rgba8i = GL_RGBA8I,
  rgba8ui = GL_RGBA8UI,
  rgba16i = GL_RGBA16I,
  rgba16ui = GL_RGBA16UI,
  rgba32i = GL_RGBA32I,
  rgba32ui = GL_RGBA32UI
};

enum class data_type : GLenum{
  deduce,
  unsigned_byte_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
  unsigned_byte_2_3_3_rev = GL_UNSIGNED_BYTE_2_3_3_REV,
  unsigned_short_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
  unsigned_short_5_6_5_rev = GL_UNSIGNED_SHORT_5_6_5_REV,
  unsigned_short_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
  unsigned_short_4_4_4_4_rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
  unsigned_short_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
  unsigned_short_1_5_5_5_rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,
  unsigned_int_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
  unsigned_int_8_8_8_8_rev = GL_UNSIGNED_INT_8_8_8_8_REV,
  unsigned_int_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
  unsigned_int_2_10_10_10_rev = GL_UNSIGNED_INT_2_10_10_10_REV
};


enum class cube_map_side : GLenum {
  positive_x = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  negative_x = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  positive_y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  negative_y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  positive_z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  negative_z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

int get_face_index(cube_map_side);

enum class min_filter : GLenum {
  nearest = GL_NEAREST,
  linear = GL_LINEAR,
  nearest_mipmap_nearest = GL_NEAREST_MIPMAP_NEAREST,
  linear_mipmap_nearest = GL_LINEAR_MIPMAP_NEAREST,
  nearest_mipmap_linear = GL_NEAREST_MIPMAP_LINEAR,
  linear_mipmap_linear = GL_LINEAR_MIPMAP_LINEAR,
};

enum class mag_filter : GLenum {
  nearest = GL_NEAREST,
  linear = GL_LINEAR
};

/** \brief Class that represents an OpenGL texture. */
class texture {
public:
  texture(texture_type typ);
  ~texture();

  void create_empty(cube_map_side side, GLsizei size, GLuint level,
                    texture_internal_format i_f);

  void create_empty(GLsizei width, GLsizei height, GLuint level,
                    texture_internal_format i_f);

  /** \brief Uploads texture data for a certain side of the cubemap texture.
   * It only works if the texture is a cubemap.
   */
  template <typename It>
  void upload(cube_map_side side, GLsizei size, GLuint level,
              texture_internal_format i_f, texture_format fmt, It begin,
              It end, data_type d_type = data_type::deduce);

  /** \brief Uploads texture data. */
  template <typename It>
  void upload(GLsizei width, GLsizei height, GLuint level,
              texture_internal_format i_f, texture_format fmt, It begin,
              It end, data_type d_type = data_type::deduce);

  /** \brief Sets the texture's minifying filter. See GL_TEXTURE_MIN_FILTER. */
  void set_parameter(min_filter mfilt);

  /** \brief Sets the texture's magnifying filter. See GL_TEXTURE_MAG_FILTER */
  void set_parameter(mag_filter mfilt);

private:
  void bind();

private:
  friend class framebuffer;
  GLuint texture_id = 0;
  const GLenum type;
};
}
}

#include "texture.inl"
