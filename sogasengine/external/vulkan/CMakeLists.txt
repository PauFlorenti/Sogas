
include(FetchContent)

if(NOT Vulkan_FOUND OR NOT DEFINED Vulkan_FOUND)
    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.3.240
    )

    set(Vulkan_INCLUDE_DIR ${Vulkan-Headers_SOURCE_DIR}/include PARENT_SCOPE)

    FetchContent_Declare(
        Vulkan-Loader
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
        GIT_TAG main
    )

    set(Vulkan_LIBRARY ${Vulkan-Loader_BINARY_DIR}/loader/${CMAKE_BUILD_TYPE}/vulkan-1.lib PARENT_SCOPE)

    set(Vulkan_FOUND true)

    message("Fetching vulkan from github source ...")
    message("Vulkan found status ${Vulkan_FOUND}")
    message("Vulkan libs: ${Vulkan_LIBRARY}")
    message("Vulkan includes: ${Vulkan_INCLUDE_DIR}")

    FetchContent_MakeAvailable(Vulkan-Headers Vulkan-Loader)
endif(NOT Vulkan_FOUND OR NOT DEFINED Vulkan_FOUND)