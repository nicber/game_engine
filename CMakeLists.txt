project(GameEngine)

cmake_minimum_required(VERSION 2.8)

set(GAME_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Options
option(GAME_ENGINE_BUILD_TESTS "Build the GameEngine test library" ON)
option(GAME_ENGINE_BUILD_DOCS "Build the GameEngine documentation" OFF)

if(GAME_ENGINE_BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(GAME_ENGINE_BUILD_DOCS)
	add_subdirectory(docs)
endif()

# Binaries

add_subdirectory(binsrc)
add_subdirectory(lib)
