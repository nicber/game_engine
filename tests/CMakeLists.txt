list(LENGTH "${GAME_ENGINE_TESTS}" TESTS_LENGTH)

if(${TESTS_LENGTH} GREATER 0)
	include_directories(../lib/include)
	include_directories(${gtest_SOURCE_DIR}/include)
	add_executable(all_tests main.cpp ${GAME_ENGINE_TESTS})
	target_link_library(all_tests game_engine gtest gtest_main)
	add_test("Run all tests" all_tests)
endif()