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
    
        VkPipeline          handle = VK_NULL_HANDLE;
        VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    };

} // Vk
} // Sogas
