#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
class VulkanPipeline;
class VulkanSwapchain;
class VulkanDevice;

class VulkanCommandBuffer
{
  public:
    explicit VulkanCommandBuffer(const VulkanDevice* device = nullptr);
    ~VulkanCommandBuffer();

    static inline VulkanCommandBuffer* ToInternal(const CommandBuffer* cmd)
    {
        return static_cast<VulkanCommandBuffer*>(cmd->internalState);
    }

    void Destroy();

    VkCommandPool              commandPools[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer            commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorPool           descriptorPools[MAX_FRAMES_IN_FLIGHT];
    std::vector<CommandBuffer> commandsToWait;

    RenderPass*                activeRenderPass   = nullptr;
    std::shared_ptr<Swapchain> swapchain          = nullptr;
    DescriptorSet*             descriptorSetBound = nullptr;
    VkSemaphore                semaphore          = VK_NULL_HANDLE;
    u32                        frameIndex;
    bool                       dirty{true};

  private:
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Sogas
