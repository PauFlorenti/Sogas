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
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer& other) = delete;

        static std::unique_ptr<Renderer::Buffer> Create(const VulkanDevice* device, Renderer::BufferDescriptor desc, void* data);

        static inline std::shared_ptr<VulkanBuffer> ToInternal(const std::unique_ptr<Renderer::Buffer>& InBuffer) {
            return std::static_pointer_cast<VulkanBuffer>(InBuffer->internal_state);
        }

        const VkBuffer*      GetHandle() const { return &handle; }
        const VkDeviceMemory GetMemory() const { return memory; }

        VkDescriptorBufferInfo descriptorInfo;

    private:
        VkBuffer        handle = VK_NULL_HANDLE;
        VkDeviceMemory  memory = VK_NULL_HANDLE;
    };
} // Vk
} // Sogas
