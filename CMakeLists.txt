
cmake_minimum_required(VERSION 3.24.2)
project(Sogas
    VERSION 1.0.0
    DESCRIPTION "Sogas Engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 

if(MSVC) 
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt")
endif()

#add_definitions(-wd4668)

add_subdirectory(sogasengine)
add_subdirectory(Sandbox)