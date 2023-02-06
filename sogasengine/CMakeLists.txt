include(FetchContent)

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

#find_package(Vulkan)

add_subdirectory(external/vulkan)
add_subdirectory(external/tinyobjloader)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

message("Vulkan libs path: ${Vulkan_LIBRARY}")
message("Vulkan include path: ${Vulkan_INCLUDE_DIR}")

add_library(${PROJECT_NAME} STATIC
    PRIVATE
    ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${glm_SOURCE_DIR}
    ${GLFW_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${tinyobjloader_SOURCE_DIR}
    )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${Vulkan_LIBRARY}
    ${GLFW_BINARY_DIR}/src/${CMAKE_BUILD_TYPE}/glfw3.lib
    spirv/lib/spirv-cross-cored)

if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
        -W4 -WX)
else()
    target_compile_options(${PROJECT_NAME}
        PRIVATE
        -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror -verbose -MTd)
endif()

target_link_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_precompile_headers(${PROJECT_NAME}
    PUBLIC public/pch.h
    PRIVATE private/sgs_pch.h)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)
file(GLOB SHADERS private/shaders/bin/*.spv)
#message(${SHADERS})
file(COPY ${SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../data/shaders)