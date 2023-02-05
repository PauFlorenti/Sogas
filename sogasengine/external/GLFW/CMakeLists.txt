
include(FetchContent)

set(GLFW_BUILD_DOCS:BOOL=OFF)
set(GLFW_BUILD_EXAMPLES:BOOL=OFF)
set(GLFW_BUILD_TESTS:BOOL=OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

FetchContent_MakeAvailable(glfw)