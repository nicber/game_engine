#pragma once

#include "opengl/buffer.h"
#include "opengl/vertex_attr.h"
#include <memory>
#include <tuple>
#include <vector>

namespace game_engine {
namespace render {
class mesh;
}
namespace opengl {
/** \brief Class for working with a vertex array object.
 */
class vertex_array_object
{
public:
  vertex_array_object( );
  ~vertex_array_object( );

  /** \brief Binds a buffer to an vertex attribute index.
  * \param F C++ type of the elements.
  * \param I number of elements in the vector passed to the program.
  * \param stride bytes between elements.
  * \param offset number of Ts that should be skipped.
  */
  template < typename T, typename F, unsigned I >
  void bind_buffer_to_attrib( std::shared_ptr< buffer< T > > buff, size_t stride, size_t offset,
                              vertex_attr attr );

  /** \brief Makes a buffer the source of indices for drawing.
    */
  void bind_buffer_to_elements_array( std::shared_ptr< buffer< unsigned short > > buff );

  void bind( ) const;

private:
  std::vector< std::shared_ptr< buffer_base > > bound_buffers;
  friend class render::mesh;
  GLuint vao_id;
};
}
}

#include "opengl/vertex_array_object.inl"
