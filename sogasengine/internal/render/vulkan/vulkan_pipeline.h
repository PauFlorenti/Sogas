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

        static VulkanPipeline& ToInternal(const Pipeline* InPipeline) {
            return *static_cast<VulkanPipeline*>(InPipeline->internalState.get());
        }
    
        VkPipeline                                  handle              = VK_NULL_HANDLE;
        VkPipelineLayout                            pipelineLayout      = VK_NULL_HANDLE;
        VkDescriptorSetLayout                       descriptorSetLayout = VK_NULL_HANDLE;

        std::vector<VkDescriptorSet>                descriptorSets;
        std::vector<VkDescriptorSetLayoutBinding>   descriptorSetLayoutBindings;
    };

} // Vk
} // Sogas
