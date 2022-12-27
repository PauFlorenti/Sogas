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

        VkShaderModule shaderModule;
    };

} // Vk
} // Sogas
