set(Boost_USE_MULTITHREADED ON)  
find_package(Boost REQUIRED COMPONENTS context) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
	set(GAME_ENGINE_DEP_LIBS ${Boost_LIBRARIES} PARENT_SCOPE)
endif()
