add_subdirectory(boost)

if(GAME_ENGINE_BUILD_TESTS)
	add_subdirectory(gtest)
endif()

set(GAME_ENGINE_DEP_LIBS ${GAME_ENGINE_DEP_LIBS} PARENT_SCOPE)
