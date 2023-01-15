#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{

    class VulkanDevice;
    
    class VulkanShader
    {
    public:
        static void Create(const VulkanDevice* device, ShaderStage stage, const char* InFilename, Shader* OutShader);

        static VulkanShader* ToInternal(const Shader* InShader) {
            return static_cast<VulkanShader*>(InShader->internalState.get());
        }

        VkShaderModule shaderModule;

        // Sets limited to 8 by now.
        std::vector<VkDescriptorSetLayoutBinding> layoutBindingsPerSet[8];
        std::vector<VkPushConstantRange> pushConstantRanges;
    };

} // Vk
} // Sogas
