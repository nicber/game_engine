#include "launcher_api.h"
#include "world.h"
#include "subsystems/render/render.h"

int main()
{
  using namespace game_engine::logic;

  game gam;
  gam.new_subsystem<subsystems::render::render_subsystem>();

  boost::thread game_thread(&start_game, &gam);
  
  auto &render_subs = gam.get<subsystems::render::render_subsystem>();
  render_subs.handle_events();
  
  game_thread.join();
}
