add_executable(launcher test.cpp)
target_link_libraries(launcher game_launcher game_engine ${GAME_ENGINE_DEP_LIBS})
