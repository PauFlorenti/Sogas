#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
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
            const SwapchainDescriptor* descriptor, 
            VulkanSwapchain* internalState);

        static VulkanSwapchain* ToInternal(const Swapchain* InSwapchain) {
            return static_cast<VulkanSwapchain*>(InSwapchain->internalState.get());
        }

        VkSwapchainKHR              swapchain   = VK_NULL_HANDLE;
        VkSurfaceKHR                surface     = VK_NULL_HANDLE;
        VkSurfaceFormatKHR          surfaceFormat;
        VkPresentModeKHR            presentMode;
        VkExtent2D                  extent;

        u32 imageIndex{0};
        std::vector<VkImage>        images;
        std::vector<VkImageView>    imageViews;
        std::vector<VkFramebuffer>  framebuffers;

        VkSemaphore swapchainStartSemaphore = VK_NULL_HANDLE;
        VkSemaphore swapchainEndSemaphore   = VK_NULL_HANDLE;
        VkFence swapchainFence[MAX_FRAMES_IN_FLIGHT];

        Texture                     texture;
        //SwapchainDescriptor         descriptor;
        RenderPass                  renderpass;
    };

} // Vk
} // Sogas
