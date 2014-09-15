#include <iostream>
#include "launcher_api.h"
#include "world.h"

void GAME_EXECUTION_FUNCTION (game_engine::logic::game *gam) {
  while(true) {
    gam->tick();
  }
}	

int main() {
	
}