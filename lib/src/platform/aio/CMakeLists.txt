if(WIN32)
  set(AIO_IMPL win32.cpp)
else()
  set(AIO_IMPL unix.cpp linux.cpp)
endif()

foreach(I IN LISTS AIO_IMPL)
  list(APPEND GAME_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${I})
endforeach(I)

set(GAME_ENGINE_SRCS ${GAME_ENGINE_SRCS} PARENT_SCOPE)
