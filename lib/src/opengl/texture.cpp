#include <opengl/texture.h>

namespace game_engine {
namespace opengl {
int
get_face_index( cube_map_side side )
{
  return static_cast< GLenum >( side ) - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
}

texture::texture( texture_type typ ) : type( static_cast< GLenum >( typ ) )
{
  glGenTextures( 1, &texture_id );
}

texture::~texture( )
{
  glDeleteTextures( 1, &texture_id );
}

void
texture::create_empty( cube_map_side side, GLsizei size, GLuint level, texture_internal_format i_f )

{
  if ( type != static_cast< GLenum >( texture_type::cube_map ) ) {
    throw std::logic_error( "this is not a cube map texture" );
  }

  bind( );

  glTexImage2D( static_cast< GLenum >( side ),
                level,
                static_cast< GLenum >( i_f ),
                size,
                size,
                0,      // border has to be 0.
                GL_RGB, // if data is NULL then there won't be a pixel transfer
                        // and the format parameter doesn't matter.
                GL_UNSIGNED_BYTE,
                nullptr );
}

void
texture::create_empty( GLsizei width, GLsizei height, GLuint level, texture_internal_format i_f )
{
  if ( type != static_cast< GLenum >( texture_type::two_d ) ) {
    throw std::logic_error( "this is not a 2D texture" );
  }

  bind( );

  glTexImage2D( static_cast< GLenum >( type ),
                level,
                static_cast< GLenum >( i_f ),
                width,
                height,
                0,      // border has to be 0.
                GL_RGB, // if data is NULL then there won't be a pixel transfer
                        // and the format parameter doesn't matter.
                GL_UNSIGNED_BYTE,
                nullptr );
}

void
texture::bind( )
{
  glBindTexture( type, texture_id );
}

void
texture::set_parameter( min_filter mfilt )
{
  auto f = static_cast< GLenum >( mfilt );
  bind( );
  glTexParameteri( texture_id, GL_TEXTURE_MIN_FILTER, f );
}

void
texture::set_parameter( mag_filter mfilt )
{
  auto f = static_cast< GLenum >( mfilt );
  bind( );
  glTexParameteri( type, GL_TEXTURE_MAG_FILTER, f );
}
}
}
