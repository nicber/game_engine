#pragma once

#include <glm/vec3.hpp>
#include <stdexcept>
#include <vector>

namespace game_engine {
namespace opengl {
namespace parser {
struct v_vt_n_indices
{
  unsigned int v_i, vt_i, n_i;
};

struct face
{
  v_vt_n_indices indices[ 3 ];
};

class inconsistent_file : public std::runtime_error
{
public:
  inconsistent_file( std::string error_msg_, std::vector< face > faces_, std::vector< glm::vec3 > vertices_,
                     std::vector< glm::vec3 > tex_coords_, std::vector< glm::vec3 > normals_ );

  const std::vector< face >& get_faces( ) const;
  const std::vector< glm::vec3 >& get_vertices( ) const;
  const std::vector< glm::vec3 >& get_tex_coords( ) const;
  const std::vector< glm::vec3 >& get_normals( ) const;

private:
  std::vector< face > faces;
  std::vector< glm::vec3 > vertices;
  std::vector< glm::vec3 > tex_coords;
  std::vector< glm::vec3 > normals;
};

class parse_error : public std::runtime_error
{
public:
  parse_error( std::string message_str_, std::string source_str_, size_t pos_ );

  /** \brief Returns the error message. */
  const std::string& error_message( ) const;

  /** \brief Returns the source string. */
  const std::string& source_string( ) const;

  /** \brief Returns an aproximate error position. */
  size_t position( ) const;

private:
  std::string message_str;
  std::string source_str;
  size_t pos;
};

/** \brief This class is return by the parse_obj_format functions.
 * It is meant to be used as a data source for VBOs or other OpenGL
 * functionalities.
 * Keep in mind that normals are not necessarily normalized.
 */
struct obj_model
{
  std::vector< unsigned int > indices;
  std::vector< glm::vec3 > vertices;
  std::vector< glm::vec3 > tex_coords;
  std::vector< glm::vec3 > normals;
};

template < typename It >
obj_model parse_obj_format( It begin, It end );

obj_model parse_obj_format( const char* str );
obj_model parse_obj_format( const std::string& str );
}
}
}

#include "obj_parser.inl"
