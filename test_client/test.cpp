#include <iostream>
#include "launcher_api.h"
#include "subsystems/render/render.h"
#include "world.h"

void GAME_EXECUTION_FUNCTION (game_engine::logic::game *gam) {
  using namespace game_engine::logic;
  using namespace game_engine::logic::subsystems::render;
  
  render_subsystem *rend_sub = nullptr;
  
  try {
    rend_sub = &gam->get<render_subsystem>();
  } catch (game::no_subsystem_found<render_subsystem> &) {
    std::cerr << "No rendering subsystem found.\n"
      "WARNING: There's no way to know that the game should stop,\n so it will"
      "         run indefinitely.";
  }
  
  
  while(!rend_sub || !rend_sub->has_exited()) {
    gam->tick();
  }
}	