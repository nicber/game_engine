include_directories(${GAME_ENGINE_ROOT}/include)

if(WIN32)
	set(PLATFORM_SPECIFIC win32.cpp)
endif()

if(UNIX)
	set(PLATFORM_SPECIFIC posix.cpp)
endif()

add_executable(game_launcher main.cpp load_library.cpp ${PLATFORM_SPECIFIC})