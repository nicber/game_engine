add_subdirectory(src)
add_subdirectory(tests)

file(GLOB_RECURSE GAME_ENGINE_HEADERS include *.h)
file(GLOB_RECURSE GAME_ENGINE_TEMPLATES include *.inl)
add_library(game_engine ${GAME_ENGINE_SRCS} ${GAME_ENGINE_HEADERS} ${GAME_ENGINE_TEMPLATES})
target_link_libraries(game_engine ${GAME_ENGINE_DEP_LIBS})