
include(FetchContent)

if(NOT Vulkan_FOUND)
    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.3.240
        FIND_PACKAGE_ARGS NAMES Vulkan
    )

    FetchContent_MakeAvailable(Vulkan-Headers)
endif(NOT Vulkan_FOUND)