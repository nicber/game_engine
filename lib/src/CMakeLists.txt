list(APPEND GAME_ENGINE_SRCS world.cpp)
list(APPEND GAME_ENGINE_SRCS subsystem.cpp)

add_library(game_engine ${GAME_ENGINE_SRCS})