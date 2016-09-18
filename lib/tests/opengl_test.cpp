#include <SFML/Window/Context.hpp>
#include <gtest/gtest.h>
#include <opengl/buffer.h>
#include <opengl/program.h>
#include <opengl/shader.h>
#include <opengl/texture.h>
#include <opengl/uniform_buffer.h>

using namespace game_engine::opengl;

TEST( OpenGLTest, BufferStoreRead )
{
  sf::Context ctx;
  glewInit( );

  size_t buffer_size = 1024;

  buffer< unsigned int > buff( buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl );

  auto it        = buff.begin( );
  unsigned int i = 0;

  for ( ; it != buff.end( ); ++it, ++i ) {
    *it = i;
  }

  EXPECT_EQ( i, buffer_size );
  i = 0;

  for ( auto& read_result : buff ) {
    unsigned int_result = read_result;
    EXPECT_EQ( int_result, i );
    ++i;
  }
}

TEST( OpenGLTest, BufferIterators )
{
  using char_buff = buffer< unsigned char >;
  char_buff buff( 1, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl );
  char_buff::buffer_iterator bit1( buff.begin( ) );
  char_buff::buffer_iterator bit2( bit1 );              // copy construct.
  char_buff::buffer_iterator bit3( std::move( bit1 ) ); // move construct.
  bit1 = bit2;                                          // copy assignment.
  bit2 = std::move( bit1 );                             // move assignment.

  char_buff::const_buffer_iterator cbit1( buff.cbegin( ) );     // construct from a const-iterator.
  char_buff::const_buffer_iterator cbit2( cbit1 );              // copy construct from a const-iterator.
  char_buff::const_buffer_iterator cbit3( std::move( cbit1 ) ); // move construct from a const-iterator.
  cbit1 = bit1;                                                 // copy assignment from a non-const iterator.
  cbit2 = std::move( bit2 );                                    // move assignment from a non-const iterator.

  cbit1 = cbit2;              // copy assignment from a const iterator.
  cbit2 = std::move( cbit3 ); // move assignment from a const iterator.

  char_buff::const_buffer_iterator cbit4( buff.begin( ) );     // construct from a non-const iterator.
  char_buff::const_buffer_iterator cbit5( bit1 );              // copy construct from a non-const iterator.
  char_buff::const_buffer_iterator cbit6( std::move( bit2 ) ); // move construct from a non-const iterator.
}

TEST( OpenGLTest, BufferResize )
{
  sf::Context ctx;
  glewInit( );

  size_t buffer_size = 1024;

  buffer< unsigned int > buff( buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl );

  auto it        = buff.begin( );
  unsigned int i = 0;

  for ( ; it != buff.end( ); ++it, ++i ) {
    *it = i;
  }

  EXPECT_EQ( i, buffer_size );
  i = 0;

  buff.resize( buffer_size * 2 );

  for ( auto& read_result : buff ) {
    unsigned int_result = read_result;
    if ( i < buffer_size ) {
      EXPECT_EQ( int_result, i );
    }
    ++i;
  }

  EXPECT_EQ( i, buffer_size * 2 );
}

TEST( OpenGLTest, BufferMove )
{
  sf::Context ctx;
  glewInit( );

  size_t buffer_size = 1024;

  buffer< unsigned int > buff( buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl );

  auto it        = buff.begin( );
  unsigned int i = 0;

  for ( ; it != buff.end( ); ++it, ++i ) {
    *it = i;
  }

  EXPECT_EQ( i, buffer_size );
  i = 0;

  buffer< unsigned int > second_buff( std::move( buff ) );

  for ( auto& read_result : second_buff ) {
    unsigned int_result = read_result;
    EXPECT_EQ( int_result, i );
    ++i;
  }

  EXPECT_EQ( i, buffer_size );
}

TEST( OpenGLTest, UniformBindingBlock )
{
  sf::Context ctx;
  glewInit( );
  std::string name = "a";
  std::vector< std::shared_ptr< const uniform_block_binding > > already_allocated_blocks;

  try {
    while ( true ) {
      already_allocated_blocks.emplace_back( get_free_uniform_block_binding( name ) );
      name += "a";
    }
  } catch ( std::runtime_error& e ) {
  }

  for ( auto& ptr : already_allocated_blocks ) {
    EXPECT_NO_THROW( get_free_uniform_block_binding( ptr->get_name( ) ) );
  }
  name += "a";
  EXPECT_THROW( get_free_uniform_block_binding( name ), std::runtime_error );
  auto max_allocated = already_allocated_blocks.size( );
  already_allocated_blocks.clear( );
  name = "a";
  for ( size_t i = 0; i < max_allocated; ++i ) {
    EXPECT_NO_THROW( { already_allocated_blocks.emplace_back( get_free_uniform_block_binding( name ) ); } );
    name += "a";
  }
  EXPECT_THROW( get_free_uniform_block_binding( name ), std::runtime_error );
}

TEST( OpenGLTest, UniformBufferCompat )
{
  sf::Context ctx;
  glewInit( );

  shader vert1( shader_type::vertex,
                "#version 140\n"
                "uniform Mat { mat4 matr; };"
                "void main() { gl_Position = matr * vec4(1,1,1,0); }",
                {} );
  shader vert2( shader_type::vertex,
                "#version 140\n"
                "uniform Mat { mat3 matr; };"
                "void main() { gl_Position = vec4(matr * vec3(1,1,1), 0); }",
                {} );
  shader vert3( shader_type::vertex,
                "#version 140\n"
                "uniform Mat { mat4 matr; vec4 vector; };"
                "void main() { gl_Position = matr * vec4(1,1,1,0) + vector; }",
                {} );

  shader frag( shader_type::fragment, "void main() { gl_FragColor = vec4(1,1,1,1); }", {} );

  program prog1( vert1, frag, {}, {}, {} );
  program prog2( vert2, frag, {}, {}, {} );
  program prog3( vert3, frag, {}, {}, {} );

  uniform_buffer buffer1( prog1, "Mat", buf_freq_access::mod_little, buf_kind_access::read_gl_write_gl );
  uniform_buffer buffer2( prog2, "Mat", buf_freq_access::mod_little, buf_kind_access::read_gl_write_gl );
  uniform_buffer buffer3( prog3, "Mat", buf_freq_access::mod_little, buf_kind_access::read_gl_write_gl );

  prog1.ubb_manager( ).add_binding( "Mat", "Mat_binding" );
  buffer1.bind_to( "Mat_binding" );
  EXPECT_TRUE( prog1.ubb_manager( ).check_compatibility( ) );
  buffer2.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog1.ubb_manager( ).check_compatibility( ) );
  buffer3.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog1.ubb_manager( ).check_compatibility( ) );

  prog2.ubb_manager( ).add_binding( "Mat", "Mat_binding" );
  buffer1.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog2.ubb_manager( ).check_compatibility( ) );
  buffer2.bind_to( "Mat_binding" );
  EXPECT_TRUE( prog2.ubb_manager( ).check_compatibility( ) );
  buffer3.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog2.ubb_manager( ).check_compatibility( ) );

  prog3.ubb_manager( ).add_binding( "Mat", "Mat_binding" );
  buffer1.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog3.ubb_manager( ).check_compatibility( ) );
  buffer2.bind_to( "Mat_binding" );
  EXPECT_FALSE( prog3.ubb_manager( ).check_compatibility( ) );
  buffer3.bind_to( "Mat_binding" );
  EXPECT_TRUE( prog3.ubb_manager( ).check_compatibility( ) );
}

TEST( OpenGLTest, UniformBufferProgramBinding )
{
  sf::Context ctx;
  glewInit( );

  shader vert( shader_type::vertex,
               "#version 140\n"
               "uniform Mat { mat4 matr; };"
               "void main() { gl_Position = matr * vec4(1,1,1,0); }",
               {} );
  shader frag( shader_type::fragment, "void main() { gl_FragColor = vec4(1,1,1,1); }", {} );
  program prog( vert, frag, {}, {}, {} );
  uniform_buffer buffer( prog, "Mat", buf_freq_access::mod_little, buf_kind_access::read_gl_write_gl );

  {
    std::vector< uniform_block_binding_handle > tmp_handles;
    std::string tmp_str( "tmp" );
    tmp_handles.emplace_back( get_free_uniform_block_binding( tmp_str + std::to_string( 0 ) ) );
    buffer.bind_to( "Mat_binding" );
    try {
      for ( size_t i = 1;; ++i ) {
        auto handle = get_free_uniform_block_binding( tmp_str + std::to_string( i ) );
        tmp_handles.emplace_back( std::move( handle ) );
      }
    } catch ( std::runtime_error& e ) {
    }
  }

  prog.ubb_manager( ).add_binding( "Mat", "Mat_binding" );

  EXPECT_TRUE( prog.ubb_manager( ).check_compatibility( ) );
}

TEST( OpenGLTest, UniformSetValue )
{
  sf::Context ctx;
  glewInit( );

  shader vert( shader_type::vertex,
               "uniform mat4 matr;"
               "void main() { gl_Position = matr * vec4(1,1,1,0); }",
               {} );

  shader frag( shader_type::fragment, "void main() { gl_FragColor = vec4(1,1,1,1); }", {} );

  program prog( vert, frag, {}, {}, {} );
  auto uni_set = prog.get_uniform( "matr" );

  EXPECT_THROW( uni_set.set( glm::vec3( ) ), game_engine::opengl::uniform_setter::type_mismatch );

  std::list< glm::mat4 > list;
  list.emplace_back( );

  uni_set.set( glm::mat4( ) );
  uni_set.set( 1, list.begin( ) );
  uni_set.set( list.begin( ), list.end( ) );

  std::vector< glm::mat4 > vec( 1 );
  uni_set.set( vec.begin( ), vec.end( ) );
}

TEST( OpenGLTest, CubeMapSideIndex )
{
  using game_engine::opengl::cube_map_side;
  EXPECT_EQ( get_face_index( cube_map_side::positive_x ), 0 );
  EXPECT_EQ( get_face_index( cube_map_side::negative_x ), 1 );
  EXPECT_EQ( get_face_index( cube_map_side::positive_y ), 2 );
  EXPECT_EQ( get_face_index( cube_map_side::negative_y ), 3 );
  EXPECT_EQ( get_face_index( cube_map_side::positive_z ), 4 );
  EXPECT_EQ( get_face_index( cube_map_side::negative_z ), 5 );
}
