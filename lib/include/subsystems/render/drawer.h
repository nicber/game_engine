#pragma once

#include "render/mesh.h"

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
class drawer
{
public:
  drawer( );
  drawer& operator+=( std::shared_ptr< const game_engine::render::mesh > m );

  using const_it = std::vector< std::shared_ptr< const game_engine::render::mesh > >::const_iterator;
  std::pair< const_it, const_it > get_meshes( ) const;

private:
  std::vector< std::shared_ptr< const game_engine::render::mesh > > meshes;
};
}
}
}
}
