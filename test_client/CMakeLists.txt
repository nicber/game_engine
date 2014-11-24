add_library(game test.cpp)

add_executable(launcher dummy.cpp)
target_link_libraries(launcher game_launcher game game_engine ${GAME_ENGINE_DEP_LIBS})
