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
        VulkanSwapchain(VulkanSwapchain&&) = delete;
        ~VulkanSwapchain();

        static bool Create(
            const VkDevice& device, 
            const VkPhysicalDevice& gpu,
            const SwapchainDescriptor* descriptor, 
            std::shared_ptr<VulkanSwapchain> internalState);

        static inline std::shared_ptr<VulkanSwapchain> ToInternal(const std::shared_ptr<Swapchain> InSwapchain) {
            return std::static_pointer_cast<VulkanSwapchain>(InSwapchain->internalState);
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

        VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderCompleteSemaphore  = VK_NULL_HANDLE;
        VkFence swapchainFence[MAX_FRAMES_IN_FLIGHT];

        Texture                     texture;
        //SwapchainDescriptor         descriptor;
        RenderPass                  renderpass;
    };

} // Vk
} // Sogas