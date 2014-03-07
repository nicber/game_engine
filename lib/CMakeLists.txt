set(GAME_ENGINE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)
foreach(SRC ${GAME_ENGINE_SRCS})
	list(APPEND GAME_ENGINE_SRCS_PREP "${CMAKE_CURRENT_SOURCE_DIR}/src/${SRC}")
endforeach()

include_directories(${GAME_ENGINE_INCLUDE})
add_library(game_engine ${GAME_ENGINE_SRCS_PREP})