#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanSwapchain;

    class VulkanCommandBuffer
    {
    public:
        VulkanCommandBuffer() = default;

        VkCommandPool   commandPools[MAX_FRAMES_IN_FLIGHT];
        VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
        u32 frameIndex;

        RenderPass* activeRenderPass;
        std::weak_ptr<VulkanSwapchain> swapchain;
    };

} // Vk
} // Sogas
