set(Boost_USE_MULTITHREADED ON)  
find_package(Boost REQUIRED COMPONENTS) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()
