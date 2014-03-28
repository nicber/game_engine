set(GAME_ENGINE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories(${GAME_ENGINE_INCLUDE})
add_subdirectory(src)
add_subdirectory(tests)