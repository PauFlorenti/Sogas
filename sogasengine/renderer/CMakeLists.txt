
file(GLOB_RECURSE
    render_sources
    private/*.cpp
    private/vulkan/*.cpp)

add_subdirectory(external/vulkan)

add_library(renderer STATIC
    PRIVATE ${render_sources})

set_target_properties(renderer PROPERTIES
    COMPILE_DEFINITIONS EXPORT)

target_include_directories(renderer
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
    ${Vulkan_INCLUDE_DIR}
    PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${glm_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${GLFW_SOURCE_DIR}/include
)

message("${CMAKE_CURRENT_SOURCE_DIR}/../external")

target_link_directories(renderer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_link_libraries(renderer
    PUBLIC
    ${GLFW_BINARY_DIR}/src/${CMAKE_BUILD_TYPE}/glfw3.lib
    PRIVATE
    #spirv/lib/spirv-cross-cored
    D:/Projects/Sogas/sogasengine/renderer/external/spirv/lib/spirv-cross-cored.lib
    ${Vulkan_LIBRARY}
    logger
)

target_precompile_headers(renderer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/internal/pch.h)