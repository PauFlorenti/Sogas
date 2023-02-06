
include(FetchContent)

if(NOT Vulkan_FOUND OR NOT DEFINED Vulkan_FOUND)

    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.3.240
    )

    FetchContent_Declare(
        Vulkan-Loader
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(Vulkan-Headers Vulkan-Loader)
    
    set(Vulkan_FOUND true)
    set(Vulkan_INCLUDE_DIR ${Vulkan-Headers_SOURCE_DIR}/include PARENT_SCOPE)
    set(Vulkan_LIBRARY ${Vulkan-Loader_BINARY_DIR}/loader/${CMAKE_BUILD_TYPE}/vulkan-1.lib PARENT_SCOPE)

endif(NOT Vulkan_FOUND OR NOT DEFINED Vulkan_FOUND)