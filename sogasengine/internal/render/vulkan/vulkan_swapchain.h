#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanDevice;

    class VulkanSwapchain
    {
    public:

        VulkanSwapchain() = default;
        VulkanSwapchain(const VulkanSwapchain&) = delete;
        VulkanSwapchain(const VulkanSwapchain&&) = delete;
        ~VulkanSwapchain();

        static bool Create(
            const VkDevice& device, 
            const VkPhysicalDevice& gpu, 
            VulkanSwapchain* internalState);

        VkSwapchainKHR      swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR        surface = VK_NULL_HANDLE;
        VkSurfaceFormatKHR  surfaceFormat;
        VkPresentModeKHR    presentMode;
        VkExtent2D          extent;

        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;

        SwapchainDescriptor descriptor;
    };

} // Vk
} // Sogas