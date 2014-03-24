project(GameEngine)

cmake_minimum_required(VERSION 2.8)

set(GAME_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Options
option(GAME_ENGINE_BUILD_TESTS "Build the GameEngine test library" ON)
option(GAME_ENGINE_BUILD_DOCS "Build the GameEngine documentation" OFF)

add_subdirectory(external)

if(GAME_ENGINE_BUILD_TESTS)
	enable_testing()
	function(define_test)
		list(APPEND ${ARGN})
		set(GAME_ENGINE_TESTS ${GAME_ENGINE_TESTS} CACHE INTERNAL "Internal variable used for tests" FORCE)
	endfunction()
else()
	function(define_test)
	endfunction(define_test)
endif()

if(GAME_ENGINE_BUILD_DOCS)
	add_subdirectory(docs)
endif()

# Binaries

add_subdirectory(binsrc)
add_subdirectory(lib)

# Tests
if(GAME_ENGINE_BUILD_TESTS)
	add_subdirectory(tests)
endif()
