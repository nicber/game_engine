#include "subsystems/render/drawer.h"

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
drawer::drawer( )
{
}

drawer&
drawer::operator+=( std::shared_ptr< const game_engine::render::mesh > m )
{
  meshes.emplace_back( std::move( m ) );
  return *this;
}

std::pair< drawer::const_it, drawer::const_it >
drawer::get_meshes( ) const
{
  return { meshes.cbegin( ), meshes.cend( ) };
}
}
}
}
}
