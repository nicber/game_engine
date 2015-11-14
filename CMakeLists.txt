project(GameEngine)

cmake_minimum_required(VERSION 2.8)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# C++14 is the default. We have to disable the min max macros defined by the
    # windows headers because they conflict with std::min and std::max.
    add_definitions("/D NOMINMAX")
    # prevent inclusion of winsock.h, winsock2.h conflicts with it.
    add_definitions("/D WIN32_LEAN_AND_MEAN")
    # we need to enable fiber-safe TLS.
    add_compile_options("/GT")
    add_compile_options("/MP")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GL /Ob2")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /LTCG")
	set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
	set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
else()
	message(SEND_ERROR "Your compiler hasn't been recognized and it hasn't been set to C++14 mode.'")
endif()

# Options
option(GAME_ENGINE_BUILD_TESTS "Build the GameEngine test library" ON)
option(GAME_ENGINE_BUILD_DOCS "Build the GameEngine documentation" OFF)

add_subdirectory(external)
# GAME_ENGINE_DEP_LIBS and GAME_ENGINE_LIBRARY_DIRS will be set after the previous line.

if(GAME_ENGINE_BUILD_TESTS)
	enable_testing()
	function(define_test TEST_NAME)
		include_directories(${gtest_SOURCE_DIR}/include)
		add_executable("test${TEST_NAME}" ${ARGN})
		target_link_libraries("test${TEST_NAME}" ${TCMALLOC_LIB} game_engine gtest gtest_main ${GAME_ENGINE_DEP_LIBS})
		add_test("${TEST_NAME}" "test${TEST_NAME}")
		list(APPEND GAME_ENGINE_TESTS ${ARGN})
		set(GAME_ENGINE_TESTS ${GAME_ENGINE_TESTS} CACHE INTERNAL "Internal variable used for tests" FORCE)
	endfunction()
else()
	function(define_test)
	endfunction(define_test)
endif()

if(GAME_ENGINE_BUILD_DOCS)
	add_subdirectory(docs)
endif()

set(GAME_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${GAME_ENGINE_ROOT}/lib/include")
include_directories(SYSTEM ${GAME_ENGINE_INCLUDE})
link_directories(${GAME_ENGINE_LIBRARY_DIRS})
add_definitions(${GAME_ENGINE_DEFINITIONS})
# Binaries

add_subdirectory(binsrc)
add_subdirectory(lib)
add_subdirectory(test_client)
