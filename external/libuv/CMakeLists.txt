find_package(PkgConfig REQUIRED)

pkg_check_modules(UV libuv)

set(GAME_ENGINE_INCLUDE ${GAME_ENGINE_INCLUDE} ${UV_INCLUDE_DIRS} PARENT_SCOPE)

if (WIN32)
    set(EXTRA_LIBS "iphlpapi.lib" "psapi.lib" "userenv.lib" "ws2_32.lib")
endif()

set(GAME_ENGINE_DEP_LIBS ${GAME_ENGINE_DEP_LIBS} ${UV_LIBRARIES} ${EXTRA_LIBS} PARENT_SCOPE)