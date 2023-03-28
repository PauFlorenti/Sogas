
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
        buffer->Upload_data_to_buffer(InDescriptor.size, InDescriptor.data);
    }

    return handle;
}

void VulkanBuffer::Upload_data_to_buffer(const u64& size, void* data)
{
    VulkanBuffer       stagingBuffer;
    VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};

    std::array<u32, 2> families = {device->GraphicsFamily, device->PresentFamily};

    if (device->GraphicsFamily != device->PresentFamily)
    {
        stagingBufferInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        stagingBufferInfo.queueFamilyIndexCount = families.size();
        stagingBufferInfo.pQueueFamilyIndices   = families.data();
    }
    else
    {
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    stagingBufferInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    stagingBufferInfo.size  = size;

    if (vkCreateBuffer(device->Handle, &stagingBufferInfo, nullptr, &stagingBuffer.buffer) != VK_SUCCESS)
    {
        SERROR("Failed to create staging buffer.");
        return;
    }

    stagingBuffer.Allocate_buffer_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(device->Handle, stagingBuffer.buffer, stagingBuffer.memory, 0);

    //stagingBuffer.SetData(data, size);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAllocateInfo.commandBufferCount = 1;
    //commandBufferAllocateInfo.commandPool        = device->resourcesCommandPool[device->GetFrameIndex()];
    commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(device->Handle, &commandBufferAllocateInfo, &cmd) != VK_SUCCESS)
    {
        SERROR("Failed to allocate command buffer.");
        return;
    }

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        SERROR("Failed to begin recording the command buffer.");
        return;
    }

    VkBufferCopy bufferCopyRegion;
    bufferCopyRegion.size      = size;
    bufferCopyRegion.srcOffset = 0;
    bufferCopyRegion.dstOffset = 0;

    vkCmdCopyBuffer(cmd, stagingBuffer.buffer, buffer, 1, &bufferCopyRegion);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        SERROR("Failed to finish recording the copy command buffer.");
        return;
    }

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;

    if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        SERROR("Failed to submit copy buffer commands.");
        return;
    }

    if (vkQueueWaitIdle(device->GraphicsQueue) != VK_SUCCESS)
    {
        SERROR("Failed waiting graphics queue to submit copy buffer commands.");
        return;
    }

    //vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
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
