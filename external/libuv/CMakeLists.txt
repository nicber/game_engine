find_package(PkgConfig REQUIRED)

if (WIN32)
    pkg_check_modules(UV libuv)
else()
    pkg_check_modules(UV REQUIRED libuv)
endif()

set(GAME_ENGINE_INCLUDE ${GAME_ENGINE_INCLUDE} ${UV_INCLUDE_DIRS} PARENT_SCOPE)

if (WIN32)
    set(EXTRA_LIBS "iphlpapi.lib" "psapi.lib" "userenv.lib" "ws2_32.lib")
endif()

set(GAME_ENGINE_LIBRARY_DIRS ${GAME_ENGINE_LIBRARY_DIRS} ${UV_LIBRARY_DIRS} PARENT_SCOPE)
set(GAME_ENGINE_DEP_LIBS ${GAME_ENGINE_DEP_LIBS} ${UV_LIBRARIES} ${EXTRA_LIBS} PARENT_SCOPE)
