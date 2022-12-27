#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanPipeline;
    class VulkanSwapchain;

    class VulkanCommandBuffer
    {
    public:
        VulkanCommandBuffer() = default;

        static VulkanCommandBuffer ToInternal(const CommandBuffer* cmd) {
            return *static_cast<VulkanCommandBuffer*>(cmd->internalState);
        }

        VkCommandPool   commandPools[MAX_FRAMES_IN_FLIGHT];
        VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
        u32 frameIndex;

        RenderPass* activeRenderPass = nullptr;
        VulkanPipeline* activePipeline = nullptr;
        std::weak_ptr<VulkanSwapchain> swapchain;
    };

} // Vk
} // Sogas
