#include "world.h"
#include "component.h"
#include "entity.h"

#include "private/sort_if_necessary.h"

namespace game_engine {
namespace logic {
const float game::max_unsorted_percentage = 0.2f;

void
swap( game& lhs, game& rhs ) no_except
{
  using std::swap;
  swap( lhs.subsystems, rhs.subsystems );

  /*
   * subsystem.parent_game has to point to the game that owns the subsystem.
   * After swapping the subsystems, their elements have parent_games that are
   * pointing to their previous game.
   */

  for ( auto& subsys_ptr : lhs.subsystems ) {
    subsys_ptr->parent_game = &lhs;
  }

  for ( auto& subsys_ptr : rhs.subsystems ) {
    subsys_ptr->parent_game = &rhs;
  }
}

game::game( game&& other ) : game( )
{
  using std::swap;
  swap( *this, other );
}

game&
game::operator=( game&& rhs ) no_except
{
  using std::swap;
  this->~game( );
  new ( this ) game( );
  swap( *this, rhs );
  return *this;
}

game::~game( )
{
  being_destroyed = true;
}

void
game::new_subsystem( std::unique_ptr< subsystem > subsys )
{
  subsystems.emplace_back( std::move( subsys ) );
  subsystems.back( )->parent_game = this;
}

void
game::tick( )
{
  for ( auto& subsys_ptr : subsystems ) {
    subsys_ptr->start_tick( );
  }

  for ( auto& subsys_ptr : subsystems ) {
    subsys_ptr->update_all( );
  }

  for ( auto& subsys_ptr : subsystems ) {
    subsys_ptr->finish_tick( );
  }
}

void
game::add_entity( std::unique_ptr< entity > ent_ptr )
{
  auto& ent = *ent_ptr;

  for ( auto& comp : ent.components ) {
    add_component( *comp.second );
  }

  ent.parent_game = this;
  entities[ typeid( ent ) ].emplace_back( std::move( ent_ptr ) );
  unsorted_number[ typeid( ent ) ]++;
  detail::sort_if_necessary( entities, unsorted_number, typeid( ent ), max_unsorted_percentage );
}

bool
game::add_component( component& comp )
{
  for ( auto& subsys_ptr : subsystems ) {
    if ( subsys_ptr->add_component( comp ) ) {
      comp.subsys = subsys_ptr.get( );
      return true;
    }
  }
  return false;
}

void
game::remove_entity( entity& ent )
{
  if ( being_destroyed ) {
    /* If the game is being destroyed we are called from an entity
     * destructor which was called by the destructor of 'entities'.
     * If we touch entities, then we might allocate memory that
     * won't be freed.
     */
    return;
  }

  auto& ent_vector = entities[ typeid( ent ) ];
  auto& unsort_num = unsorted_number[ typeid( ent ) ];

  auto it =
    std::lower_bound( ent_vector.begin( ),
                      ent_vector.end( ) - unsort_num,
                      &ent,
                      []( std::unique_ptr< entity >& lhs, entity* rhs ) { return lhs.get( ) < rhs; } );
  if ( it == ent_vector.end( ) - unsort_num ) {
    std::find_if(
      it, ent_vector.end( ), [&ent]( std::unique_ptr< entity >& ptr ) { return ptr.get( ) == &ent; } );
  }

  if ( it == ent_vector.end( ) ) {
    throw std::invalid_argument( "Entity not in the game." );
  }

  unsort_num--;
  ent_vector.erase( it );
  detail::sort_if_necessary( entities, unsorted_number, typeid( ent ), max_unsorted_percentage );
}
}
}