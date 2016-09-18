#include "render/mesh.h"

namespace game_engine {
namespace render {
mesh::mesh( std::shared_ptr< const opengl::program > prog_,
            std::shared_ptr< const opengl::vertex_array_object > vao_, before_draw_function func_,
            size_t count_, size_t base_index_, size_t base_vertex_, size_t instance_count_,
            size_t base_instance_ )
  : prog( std::move( prog_ ) )
  , vao( std::move( vao_ ) )
  , before_draw_func( std::move( func_ ) )
  , count( count_ )
  , base_index( base_index_ )
  , base_vertex( base_vertex_ )
  , instance_count( instance_count_ )
  , base_instance( base_instance_ )
{
}

void
mesh::init( const mesh::uni_buff_vector& uni_buffs_ )
{
  uni_buffs_with_conn.reserve( uni_buffs_.size( ) );
  for ( auto& u_buff_dep : uni_buffs_ ) {
    boost::signals2::scoped_connection scoped_conn;
    if ( u_buff_dep.should_reset_delta ) {
      auto shared_this_ptr = shared_from_this( );
      scoped_conn          = u_buff_dep.buffer_ptr->on_change_connect(
        opengl::uniform_buffer::on_change_slot( [mesh_wptr = std::weak_ptr< mesh >( shared_this_ptr )]( ) {
          auto mesh_ptr = mesh_wptr.lock( );
          assert( mesh_ptr &&
                  "it should not have been called by the signal "
                  "if it is expired" );
          // by setting it to 0 we make the drawer thread start counting
          // delta time since the next update that occurs.
          mesh_ptr->absolute_last_update_time.set_zero( );
        } )
          .track_foreign( std::move( shared_this_ptr ) ) );
    }
    uni_buffs_with_conn.emplace_back(
      uni_buff_connection{ std::move( u_buff_dep.buffer_ptr ), std::move( scoped_conn ) } );
  }
}

bool
mesh::operator<( const mesh& rhs ) const
{
  if ( vao->vao_id < rhs.vao->vao_id ) {
    return true;
  } else if ( vao->vao_id > rhs.vao->vao_id ) {
    return false;
  } else { // vao->vao_id == rhs.vao->vao_id
    return prog->program_id < rhs.prog->program_id;
  }
}

void
mesh::draw( logic::time delta_time ) const
{
  vao->bind( );
  prog->bind( );

  if ( before_draw_func ) {
    before_draw_func( delta_time );
  }

  for ( const auto& u_buff_conn : uni_buffs_with_conn ) {
    u_buff_conn.buffer_ptr->rebind( );
  }

  if ( !prog->ubb_manager( ).check_compatibility( ) ) {
    throw std::runtime_error( "incompatible program and uniform buffer" );
  }

  auto index = reinterpret_cast< const void* >( base_index );
  glDrawElementsInstancedBaseVertexBaseInstance( GL_TRIANGLES,      // mode
                                                 count,             // number of prims
                                                 GL_UNSIGNED_SHORT, // type of indices
                                                 index,             // offset in indices array
                                                 instance_count,
                                                 base_vertex,
                                                 base_instance );
}

struct mesh_constructor : public mesh
{
  mesh_constructor( std::shared_ptr< const opengl::program > prog_,
                    std::shared_ptr< const opengl::vertex_array_object > vao_, before_draw_function func_,
                    size_t count_, size_t base_index_, size_t base_vertex_, size_t instance_count_,
                    size_t base_instance_ )
    : mesh( std::move( prog_ ), std::move( vao_ ), std::move( func_ ), count_, base_index_, base_vertex_,
            instance_count_, base_instance_ )
  {
  }

  void init( const uni_buff_vector& uni_buffs_ )
  {
    mesh::init( uni_buffs_ );
  }
};

std::shared_ptr< mesh >
create_mesh( std::shared_ptr< const opengl::program > prog_,
             std::shared_ptr< const opengl::vertex_array_object > vao_, before_draw_function func_,
             const mesh::uni_buff_vector& uni_buffs_, size_t count_, size_t base_index_, size_t base_vertex_,
             size_t instance_count_, size_t base_instance_ )
{
  auto ptr = std::make_shared< mesh_constructor >( std::move( prog_ ),
                                                   std::move( vao_ ),
                                                   std::move( func_ ),
                                                   count_,
                                                   base_index_,
                                                   base_vertex_,
                                                   instance_count_,
                                                   base_instance_ );
  ptr->init( uni_buffs_ );
  return ptr;
}
}
}
