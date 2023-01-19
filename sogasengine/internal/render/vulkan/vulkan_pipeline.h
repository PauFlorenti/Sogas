#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline() = default;
        VulkanPipeline(const VulkanPipeline&) = delete;
        VulkanPipeline(VulkanPipeline&&) = delete;
        const VulkanPipeline& operator=(const VulkanPipeline& other) = delete;

        static void Create(const VulkanDevice* device, const PipelineDescriptor* descriptor, Pipeline* pipeline, RenderPass* renderpass = nullptr);

        static inline std::shared_ptr<VulkanPipeline> ToInternal(const Pipeline* InPipeline) {
            return std::static_pointer_cast<VulkanPipeline>(InPipeline->internalState);
        }

        void CreateDescriptorSets();
    
        // TODO rethink this, probably make them private ...
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
