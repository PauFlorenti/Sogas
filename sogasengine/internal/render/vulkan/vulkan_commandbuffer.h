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

        RenderPass*         activeRenderPass = nullptr;
        //VulkanPipeline*     activePipeline   = nullptr;
        Swapchain*          swapchain        = nullptr;
        DescriptorSet*      descriptorSetBound = nullptr;

        bool                            dirty{true};
        VkDescriptorPool                descriptorPools[MAX_FRAMES_IN_FLIGHT];
        std::vector<VkDescriptorSet>    descriptorSets;
    };

} // Vk
} // Sogas
