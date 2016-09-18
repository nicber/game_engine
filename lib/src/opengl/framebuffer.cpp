#include "opengl/framebuffer.h"
#include <boost/functional/hash.hpp>
#include <cassert>

namespace game_engine {
namespace opengl {
attachment
init_att( GLenum constant )
{
  attachment r;
  r.gl_constant = constant;
  return r;
}

attachment
attachment::color_attachmenti( size_t i )
{
  return init_att( GL_COLOR_ATTACHMENT0 + i );
}

attachment attachment::depth_attachment = init_att( GL_DEPTH_ATTACHMENT );

attachment attachment::stencil_attachment = init_att( GL_STENCIL_ATTACHMENT );

attachment attachment::depth_stencil_attachment = init_att( GL_DEPTH_STENCIL_ATTACHMENT );

bool
attachment::operator==( const attachment& rhs ) const
{
  return gl_constant == rhs.gl_constant;
}

bool
attachment::operator!=( const attachment& rhs ) const
{
  return gl_constant != rhs.gl_constant;
}

GLenum
attachment::get_gl_constant( ) const
{
  return gl_constant;
}

size_t
hash_value( const attachment& rnd_at )
{
  return boost::hash_value( rnd_at.gl_constant );
}

static GLuint read_and_draw_bound = 0;
static GLuint draw_bound          = 0;
static GLuint read_bound          = 0;

framebuffer::framebuffer( )
{
  glGenFramebuffers( 1, &framebuffer_id );
}

framebuffer::~framebuffer( )
{
  if ( read_and_draw_bound == framebuffer_id ) {
    read_and_draw_bound = 0;
  } else if ( draw_bound == framebuffer_id ) {
    draw_bound = 0;
  } else if ( read_bound == framebuffer_id ) {
    read_bound = 0;
  }
  glDeleteFramebuffers( 1, &framebuffer_id );
}

void
framebuffer::bind_to( framebuffer::render_target target )
{
  glBindFramebuffer( static_cast< GLenum >( target ), framebuffer_id );
  switch ( target ) {
    case render_target::read_and_draw:
      read_and_draw_bound = framebuffer_id;
      break;
    case render_target::draw:
      draw_bound = framebuffer_id;
      break;
    case render_target::read:
      read_bound = framebuffer_id;
      break;
    default:
      assert( false );
  }
}

void
framebuffer::attach( attachment rnd_at, const std::shared_ptr< const renderbuffer >& ptr )
{
  add_attachment( ptr, rnd_at );
  do_attach( rnd_at, *ptr );
}

void
framebuffer::attach( attachment rnd_at, const renderbuffer& rbuffer )
{
  remove_attachment_if_not_eq( &rbuffer, rnd_at );
  do_attach( rnd_at, rbuffer );
}

void
framebuffer::attach( attachment att, size_t level, const std::shared_ptr< const texture >& ptr )
{
  add_attachment( ptr, att );
  do_attach( att, *ptr, level );
}

void
framebuffer::attach( attachment att, size_t level, const texture& tex )
{
  remove_attachment_if_not_eq( &tex, att );
  do_attach( att, tex, level );
}

void
framebuffer::attach( attachment att, size_t level, cube_map_side side,
                     const std::shared_ptr< const texture >& ptr )
{
  add_attachment( ptr, att );
  do_attach( att, *ptr, level, side );
}

void
framebuffer::attach( attachment att, size_t level, cube_map_side side, const texture& tex )
{
  remove_attachment_if_not_eq( &tex, att );
  do_attach( att, tex, level, side );
}

bool
framebuffer::get_bind( render_target& ret )
{
  if ( read_and_draw_bound == framebuffer_id ) {
    ret = render_target::read_and_draw;
    return true;
  } else if ( draw_bound == framebuffer_id ) {
    ret = render_target::draw;
    return true;
  } else if ( read_bound == framebuffer_id ) {
    ret = render_target::read;
    return true;
  }
  return false;
}

framebuffer::render_target
framebuffer::bind_if_not_bound( )
{
  render_target framebuffer_binding;
  if ( !get_bind( framebuffer_binding ) ) {
    framebuffer_binding = render_target::read_and_draw;
    bind_to( framebuffer_binding );
  }
  return framebuffer_binding;
}

void
framebuffer::add_attachment( const std::weak_ptr< const void > ptr, attachment att )
{
  if ( att != attachment::depth_stencil_attachment ) {
    attachments[ att ] = std::move( ptr );
  } else {
    attachments[ attachment::depth_attachment ]   = std::move( ptr );
    attachments[ attachment::stencil_attachment ] = std::move( ptr );
  }
}

void
framebuffer::remove_attachment_if_not_eq( const void* image_ptr, attachment att )
{
  auto remove_attachment = [&]( attachment att ) {
    auto it = attachments.find( att );
    if ( it != attachments.end( ) && it->second.lock( ).get( ) != image_ptr ) {
      attachments.erase( it );
    }
  };
  if ( att != attachment::depth_stencil_attachment ) {
    remove_attachment( att );
  } else {
    remove_attachment( attachment::stencil_attachment );
    remove_attachment( attachment::depth_attachment );
  }
}

void
framebuffer::do_attach( attachment rnd_at, const renderbuffer& rbuffer )
{
  auto framebuffer_binding = bind_if_not_bound( );
  glFramebufferRenderbuffer( static_cast< GLenum >( framebuffer_binding ),
                             rnd_at.gl_constant,
                             GL_RENDERBUFFER,
                             rbuffer.renderbuffer_id );
}

void
framebuffer::do_attach( attachment tex_at, const texture& tex, size_t level )
{
  auto framebuffer_binding = bind_if_not_bound( );
  glFramebufferTexture(
    static_cast< GLenum >( framebuffer_binding ), tex_at.gl_constant, tex.texture_id, level );
}

void
framebuffer::do_attach( attachment tex_at, const texture& tex, size_t level, cube_map_side side )
{
  auto framebuffer_binding = bind_if_not_bound( );
  auto layer               = get_face_index( side );
  glFramebufferTextureLayer(
    static_cast< GLenum >( framebuffer_binding ), tex_at.gl_constant, tex.texture_id, level, layer );
}
}
}
