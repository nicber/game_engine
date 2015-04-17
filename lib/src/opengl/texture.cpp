#include <opengl/texture.h>

namespace game_engine {
namespace opengl {
int get_face_index(cube_map_side side)
{
  return static_cast<GLenum>(side) - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
}

texture::texture(texture_type typ)
 :type(static_cast<GLenum>(typ))
{
  glGenTextures(1, &texture_id);
}

texture::~texture()
{
  glDeleteTextures(1, &texture_id);
}

void texture::bind()
{
  glBindTexture(type, texture_id);
}

void texture::set_parameter(min_filter mfilt) {
  auto f = static_cast<GLenum>(mfilt);
  bind();
  glTexParameteri(texture_id, GL_TEXTURE_MIN_FILTER, f);
}

void texture::set_parameter(mag_filter mfilt) {
  auto f = static_cast<GLenum>(mfilt);
  bind();
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, f);
}

}
}
