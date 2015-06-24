list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/component.cpp)
list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/entity.cpp)
list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/subsystem.cpp)
list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/time_access.cpp)
list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/world.cpp)

add_subdirectory(logging)
add_subdirectory(opengl)
add_subdirectory(platform)
add_subdirectory(render)
add_subdirectory(subsystems)
add_subdirectory(thr_queue)

set(GAME_ENGINE_SRCS ${GAME_ENGINE_SRCS} PARENT_SCOPE)
