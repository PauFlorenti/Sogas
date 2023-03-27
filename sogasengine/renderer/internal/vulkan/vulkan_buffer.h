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
    friend class VulkanTexture;

  public:
    explicit VulkanBuffer(const VulkanDevice* device = nullptr);
    VulkanBuffer(const VulkanBuffer&)                  = delete;
    VulkanBuffer(VulkanBuffer&&)                       = delete;
    VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
    ~VulkanBuffer();

    void Release() override;

    static BufferHandle Create(VulkanDevice* InDevice, const BufferDescriptor& InDescriptor);

    void SetData(void* data, const u64& size, const u64& offset = 0) override;

    VkDescriptorBufferInfo descriptorInfo;

    VkBufferUsageFlags usage_flags   = 0;
    u32                size          = 0;
    u32                global_offset = 0; // Offset into global constant, if dynamic.

    VkBuffer     buffer = VK_NULL_HANDLE;
    BufferHandle handle;
    std::string  name;

  private:
    void Upload_data_to_buffer(const u64& size, void* data);
    void Allocate_buffer_memory(VkMemoryPropertyFlags memoryPropertyFlags);

    void*               mapdata = nullptr;
    const VulkanDevice* device  = nullptr;
    VkDeviceMemory      memory  = VK_NULL_HANDLE;
    VkDeviceSize        device_size;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
