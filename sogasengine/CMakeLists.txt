include(FetchContent)

project(SogasEngine LANGUAGES CXX)

file(GLOB_RECURSE
    SOURCES
    private/*.cpp
    private/entity/*.cpp
    private/modules/*.cpp
    private/render/*.cpp
    private/resources/*.cpp
    private/handle/*.cpp)

add_subdirectory(external/tinyobjloader)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

add_subdirectory(logger)
add_subdirectory(renderer)

add_library(${PROJECT_NAME} STATIC
    PRIVATE
    ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/public
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${glm_SOURCE_DIR}
    ${GLFW_SOURCE_DIR}/include
    PRIVATE
    .
    #${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/private
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${tinyobjloader_SOURCE_DIR}
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${GLFW_BINARY_DIR}/src/${CMAKE_BUILD_TYPE})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    logger
    renderer
    ${GLFW_BINARY_DIR}/src/${CMAKE_BUILD_TYPE}/glfw3.lib
)

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
file(COPY ${SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../data/shaders)