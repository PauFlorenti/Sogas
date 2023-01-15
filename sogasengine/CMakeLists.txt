
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
        ${CMAKE_CURRENT_SOURCE_DIR}/private
        ${CMAKE_CURRENT_SOURCE_DIR}/internal)

target_link_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Vulkan_LIBRARIES}
        GLFW/lib/glfw3
        spirv/lib/spirv-cross-cored)
        #spirv/lib/spirv-cross-reflect
        #spirv/lib/spirv-cross-glsl
        #spirv/lib/spirv-cross-cpp
        #spirv/lib/spirv-cross-core)

target_precompile_headers(${PROJECT_NAME} 
    PUBLIC public/pch.h PRIVATE private/sgs_pch.h)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)
file(GLOB SHADERS private/shaders/bin/*.spv)
message(${SHADERS})
file(COPY ${SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../data/shaders)