#pragma once
#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanDevice;
    class VulkanDescriptorSet
    {
    public:
        VulkanDescriptorSet() = default;

        static void Create(const VulkanDevice* InDevice, DescriptorSet* InDescriptorSet, const Pipeline* InPipeline);

        static VulkanDescriptorSet* ToInternal(const DescriptorSet* InDescriptorSet) {
            return static_cast<VulkanDescriptorSet*>(InDescriptorSet->internalState.get());
        }

        void BindDescriptor(VkCommandBuffer cmd) const;

        const VkDescriptorSet& GetDescriptorSet() const { return descriptorSet; }

    private:
        VkPipelineBindPoint pipelineBindPoint;
        VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool    descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet     descriptorSet = VK_NULL_HANDLE;
    };
} // Vk
} // Sogas
