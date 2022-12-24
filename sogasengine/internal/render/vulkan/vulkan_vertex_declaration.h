#pragma once

#include <vulkan/vulkan.h>

namespace Sogas 
{
namespace Vk 
{
    struct CVulkanVertexDeclaration
    {
        std::string name;
        const VkVertexInputAttributeDescription* layout = VK_NULL_HANDLE;
        u32 size = 0;

        CVulkanVertexDeclaration(const char* name, const VkVertexInputAttributeDescription* newLayout, const u32 size);
    };

    const CVulkanVertexDeclaration* GetVertexDeclaration(const std::string& name);
} // Vk
} // Sogas
