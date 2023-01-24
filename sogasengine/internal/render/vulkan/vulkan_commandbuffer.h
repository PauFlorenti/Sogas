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

        static inline VulkanCommandBuffer* ToInternal(const CommandBuffer* cmd) {
            return static_cast<VulkanCommandBuffer*>(cmd->internalState);
        }

        VkCommandPool   commandPools[MAX_FRAMES_IN_FLIGHT];
        VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
        u32 frameIndex;

        RenderPass*         activeRenderPass    = nullptr;
        std::shared_ptr<Swapchain> swapchain = nullptr;
        DescriptorSet*      descriptorSetBound  = nullptr;

        VkSemaphore semaphore = VK_NULL_HANDLE;

        std::vector<CommandBuffer> commandsToWait;

        bool                            dirty{true};
        VkDescriptorPool                descriptorPools[MAX_FRAMES_IN_FLIGHT];
        std::vector<VkDescriptorSet>    descriptorSets;
    };

} // Vk
} // Sogas
