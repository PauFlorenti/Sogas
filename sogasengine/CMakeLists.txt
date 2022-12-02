
project(SogasEngine LANGUAGES CXX)

file(GLOB_RECURSE 
    SOURCES 
        private/*.cpp 
        private/entity/*.cpp
        private/modules/*.cpp 
        private/render/*.cpp 
        private/render/vulkan/*.cpp 
        private/resources/*.cpp
        private/handle/*.cpp)

find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} STATIC 
    PRIVATE 
        ${SOURCES})

if(MSVC) 
    target_compile_options(${PROJECT_NAME} 
        PRIVATE 
            -W4 -WX)
else()
    target_compile_options(${PROJECT_NAME} 
        PRIVATE 
            -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror -verbose -MTd)
endif()

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${Vulkan_INCLUDE_DIRS}
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Vulkan_LIBRARIES}
        GLFW/lib/glfw3)

target_precompile_headers(${PROJECT_NAME} 
    PUBLIC public/pch.h)