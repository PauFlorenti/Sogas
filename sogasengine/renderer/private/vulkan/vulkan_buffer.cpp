
#include "vulkan/vulkan_buffer.h"
#include "device_resource_pool.h"
#include "device_resources.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

static VkBufferUsageFlags ConvertUsage(BufferUsage InUsage)
{
    switch (InUsage)
    {
        case BufferUsage::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BufferUsage::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferUsage::UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferUsage::TRANSFER_DST:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferUsage::TRANSFER_SRC:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        default:
        case BufferUsage::UNDEFINED:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
    };
}

BufferHandle VulkanBuffer::Create(VulkanDevice* InDevice, const BufferDescriptor& InDescriptor)
{
    BufferHandle handle = {InDevice->buffers.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanBuffer* buffer  = static_cast<VulkanBuffer*>(InDevice->buffers.AccessResource(handle.index));
    buffer->device        = InDevice;
    buffer->name          = InDescriptor.name;
    buffer->usage_flags   = ConvertUsage(InDescriptor.usage);
    buffer->size          = InDescriptor.size;
    buffer->global_offset = 0;
    buffer->usage_type    = InDescriptor.type;
    buffer->handle        = handle;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.usage              = VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer->usage_flags;
    buffer_info.size               = InDescriptor.size > 0 ? InDescriptor.size : 1;

    SASSERT(vkCreateBuffer(InDevice->Handle, &buffer_info, nullptr, &buffer->buffer) == VK_SUCCESS);

    VkMemoryPropertyFlags memory_property_flags =
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    buffer->Allocate_buffer_memory(memory_property_flags);

    vkBindBufferMemory(InDevice->Handle, buffer->buffer, buffer->memory, 0);

    if (InDescriptor.data != nullptr)
    {
        void* data;
        vkMapMemory(InDevice->Handle, buffer->memory, 0, InDescriptor.size, 0, &data);
        memcpy(data, InDescriptor.data, InDescriptor.size);
        vkUnmapMemory(InDevice->Handle, buffer->memory);
    }

    return handle;
}

void VulkanBuffer::Allocate_buffer_memory(VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->Handle, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize       = memoryRequirements.size;
    allocateInfo.memoryTypeIndex      = device->FindMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

    if (vkAllocateMemory(device->Handle, &allocateInfo, nullptr, &memory))
    {
        SERROR("Failed to allocate buffer memory.");
    }
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
