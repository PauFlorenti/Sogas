#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanDevice;

    class VulkanBuffer
    {
    public:
        VulkanBuffer() = default;
        VulkanBuffer(VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&&) = delete;

        static void Create(
            const VulkanDevice& device, 
            const GPUBufferDescriptor* desc, 
            void* data, 
            GPUBuffer* buffer);

        const VkBuffer* GetHandle() const { return &handle; }

    private:
        VkBuffer handle         = VK_NULL_HANDLE;
        VkDeviceMemory memory   = VK_NULL_HANDLE;
    };

} // Vk
} // Sogas
