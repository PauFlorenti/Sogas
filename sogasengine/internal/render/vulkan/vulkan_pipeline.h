#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanPipeline
    {
    public:
        static void Create(const VulkanDevice* device, const PipelineDescriptor* descriptor, Pipeline* pipeline, RenderPass* renderpass = nullptr);

        static VulkanPipeline* ToInternal(const Pipeline* InPipeline) {
            return static_cast<VulkanPipeline*>(InPipeline->internalState.get());
        }

        void CreateDescriptorSets();
    
        VkPipeline                                  handle              = VK_NULL_HANDLE;
        VkPipelineLayout                            pipelineLayout      = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout>          descriptorSetLayouts;
        std::vector<VkDescriptorSetLayoutBinding>   descriptorSetLayoutBindingsPerSet[8];
        std::vector<DescriptorSet>                  descriptorSets[MAX_FRAMES_IN_FLIGHT];
    private:
        const VulkanDevice*                         device              = nullptr;
    };

} // Vk
} // Sogas
