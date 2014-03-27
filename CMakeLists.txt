project(GameEngine)

cmake_minimum_required(VERSION 2.8)

set(GAME_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# It's the default.
else()
	message(SEND_ERROR "Your compiler hasn't been recognized and it hasn't been set to C++11 mode.'")
endif()

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
