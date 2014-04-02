include_directories(${GAME_ENGINE_ROOT}/include)

if(WIN32)
	set(PLATFORM_SOURCE win32.cpp)
endif()

if(UNIX)
	set(PLATFORM_SOURCE posix.cpp)
	set(PLATFORM_LIBRARY dl)
endif()

add_executable(game_launcher main.cpp load_library.cpp ${PLATFORM_SOURCE})
target_link_libraries(game_launcher ${PLATFORM_LIBRARY})