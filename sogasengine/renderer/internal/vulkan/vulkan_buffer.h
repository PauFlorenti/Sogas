#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;

class VulkanBuffer
{
    friend class VulkanTexture;

  public:
    static BufferHandle Create(VulkanDevice* InDevice, const BufferDescriptor& InDescriptor);

    VkBuffer       buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    BufferHandle   handle;

    VkBufferUsageFlags usage_flags   = 0;
    BufferType         usage_type    = BufferType::Static;
    u32                size          = 0;
    u32                global_offset = 0; // Offset into global constant, if dynamic.

    std::string name;

  private:
    void Allocate_buffer_memory(VkMemoryPropertyFlags memoryPropertyFlags);

    const VulkanDevice* device = nullptr;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
