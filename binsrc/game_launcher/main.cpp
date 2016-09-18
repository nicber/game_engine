#include "launcher_api.h"
#include "logging/file_loader.h"
#include "subsystems/render/render.h"
#include "world.h"

int
main( )
{
  using namespace game_engine;
  using namespace game_engine::logic;
  logging::set_default_for_all( logging::policy::enable );
  try {
    logging::load_policies_from_file( "log_policies" );
  } catch ( const logging::parse_exception& e ) {
    std::cerr << "Error loading logging policies from file: " << e.what( ) << '\n';
  }

  game gam;
  gam.new_subsystem< subsystems::render::render_subsystem >( );

  boost::thread game_thread( &start_game, &gam );

  auto& render_subs = gam.get< subsystems::render::render_subsystem >( );
  render_subs.handle_events( );

  game_thread.join( );
}
