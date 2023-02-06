#pragma once

#include "vulkan_types.h"
#include "device_buffer.h"

namespace Sogas
{
namespace Vk
{
    class VulkanDevice;
    
    class VulkanBuffer final : public Renderer::DeviceBuffer
    {
    public:
        explicit VulkanBuffer(const VulkanDevice* device = nullptr);
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
        ~VulkanBuffer();

        static std::unique_ptr<Renderer::Buffer> Create(const VulkanDevice* device, Renderer::BufferDescriptor desc, void* data);

        static inline std::shared_ptr<VulkanBuffer> ToInternal(const std::shared_ptr<Renderer::DeviceBuffer> InBuffer) {
            return std::dynamic_pointer_cast<VulkanBuffer>(InBuffer);
        }

        const VkBuffer*      GetHandle() const { return &handle; }
        const VkDeviceMemory GetMemory() const { return memory; }
        
        const VkDescriptorBufferInfo GetDescriptor() const { return descriptorInfo; }

        VkDescriptorBufferInfo descriptorInfo;
    private:
        const VulkanDevice*     gpu_device = nullptr;
        VkBuffer                handle = VK_NULL_HANDLE;
        VkDeviceMemory          memory = VK_NULL_HANDLE;
    };
} // Vk
} // Sogas
