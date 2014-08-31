#pragma once
#include "world.h"
#define GAME_EXECUTION_FUNCTION_NAME "execute_game"
#define GAME_EXECUTION_FUNCTION execute_game

extern "C" {
void GAME_EXECUTION_FUNCTION(game_engine::logic::game *gam);
}

#define GAME_EXECUTION_TYPE decltype(GAME_EXECUTION_FUNCTION)