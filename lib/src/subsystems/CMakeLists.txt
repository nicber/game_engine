add_subdirectory(ai)
add_subdirectory(render)

list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/time.cpp)
set(GAME_ENGINE_SRCS ${GAME_ENGINE_SRCS} PARENT_SCOPE)
