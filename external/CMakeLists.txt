add_subdirectory(boost)
add_subdirectory(glew)
add_subdirectory(glm)
add_subdirectory(libuv)
add_subdirectory(sfml)

if(GAME_ENGINE_BUILD_TESTS)
	add_subdirectory(gtest)
endif()

set(GAME_ENGINE_INCLUDE ${GAME_ENGINE_INCLUDE} PARENT_SCOPE)
set(GAME_ENGINE_DEFINITIONS ${GAME_ENGINE_DEFINITIONS} PARENT_SCOPE)
set(GAME_ENGINE_DEP_LIBS ${GAME_ENGINE_DEP_LIBS} PARENT_SCOPE)
