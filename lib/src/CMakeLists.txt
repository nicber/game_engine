list(APPEND GAME_ENGINE_SRCS entity.cpp)
list(APPEND GAME_ENGINE_SRCS subsystem.cpp)
list(APPEND GAME_ENGINE_SRCS time_access.cpp)
list(APPEND GAME_ENGINE_SRCS world.cpp)

add_subdirectory(subsystems)
add_subdirectory(platform)

add_library(game_engine ${GAME_ENGINE_SRCS})