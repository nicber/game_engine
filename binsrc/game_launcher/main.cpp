#include "launcher_api.h"
#include "load_library.h"
#include "world.h"
#include "subsystems/render/render.h"

int main()
{
  using namespace game_launcher;
  using namespace game_engine::logic;
  library lib("game");

  game gam;
  gam.new_subsystem<subsystems::render::render_subsystem>();

  auto gam_exec_func = lib.get<GAME_EXECUTION_TYPE>(GAME_EXECUTION_FUNCTION_NAME);

  boost::thread game_thread(gam_exec_func, &gam);
  
  auto &render_subs = gam.get<subsystems::render::render_subsystem>();
  render_subs.handle_events();
	
	game_thread.join();
}
