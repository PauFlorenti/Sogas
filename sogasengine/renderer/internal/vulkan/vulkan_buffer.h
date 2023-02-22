#pragma once

#include "device_buffer.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;

class VulkanBuffer final : public Renderer::DeviceBuffer
{
  public:
    explicit VulkanBuffer(const VulkanDevice* device = nullptr);
    VulkanBuffer(const VulkanBuffer&)                  = delete;
    VulkanBuffer(VulkanBuffer&&)                       = delete;
    VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
    ~VulkanBuffer();

    void Release() override;

    static std::unique_ptr<Renderer::Buffer>
    Create(const VulkanDevice* device, Renderer::BufferDescriptor desc, void* data);

    static inline std::shared_ptr<VulkanBuffer> ToInternal(const std::shared_ptr<Renderer::DeviceBuffer> InBuffer)
    {
        return std::dynamic_pointer_cast<VulkanBuffer>(InBuffer);
    }

    const VkBuffer*      GetHandle() const { return &handle; }
    const VkDeviceMemory GetMemory() const { return memory; }
    void                 SetData(void* data, const u64& size, const u64& offset = 0) override;

    const VkDescriptorBufferInfo GetDescriptor() const { return descriptorInfo; }

    VkDescriptorBufferInfo descriptorInfo;

  private:
    void Upload_data_to_buffer(const u64& size, void* data);
    void Allocate_buffer_memory(VkMemoryPropertyFlags memoryPropertyFlags);

    void*               mapdata = nullptr;
    const VulkanDevice* device  = nullptr;
    VkBuffer            handle  = VK_NULL_HANDLE;
    VkDeviceMemory      memory  = VK_NULL_HANDLE;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
