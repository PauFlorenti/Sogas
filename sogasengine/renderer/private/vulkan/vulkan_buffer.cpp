
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

VulkanBuffer::VulkanBuffer(const VulkanDevice* device)
    : device(device)
{
    SASSERT(device != nullptr);
}

VulkanBuffer::~VulkanBuffer()
{
    Release();
}

void VulkanBuffer::Release()
{
    SASSERT(device != nullptr);
    vkDestroyBuffer(device->Handle, handle, nullptr);
    vkFreeMemory(device->Handle, memory, nullptr);
}

BufferHandle VulkanBuffer::Create(const BufferDescriptor& InDescriptor)
{
    return BufferHandle();
}

std::unique_ptr<Renderer::Buffer>
VulkanBuffer::Create(const VulkanDevice* device, Renderer::BufferDescriptor desc, void* data)
{
    std::unique_ptr<Renderer::Buffer> buffer        = std::make_unique<Renderer::Buffer>(desc);
    auto                              internalState = std::make_shared<VulkanBuffer>(device);
    buffer->device_buffer                           = internalState;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size               = buffer->ByteSize();

    if (desc.binding == Renderer::BufferBindingPoint::Vertex)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (desc.binding == Renderer::BufferBindingPoint::Index)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (desc.binding == Renderer::BufferBindingPoint::Uniform)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
    if (desc.usage == Renderer::BufferUsage::TRANSFER_DST)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    if (desc.usage == Renderer::BufferUsage::TRANSFER_SRC)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    u32 families[2] = {device->GraphicsFamily, device->PresentFamily};
    if (device->GraphicsFamily != device->PresentFamily)
    {
        buffer_info.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        buffer_info.queueFamilyIndexCount = 2;
        buffer_info.pQueueFamilyIndices   = families;
    }
    else
    {
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateBuffer(device->Handle, &buffer_info, nullptr, &internalState->handle) != VK_SUCCESS)
    {
        SERROR("Failed to create buffer.");
        buffer->device_buffer.reset();
        return buffer;
    }

    internalState->Allocate_buffer_memory(memoryPropertyFlags);

    vkBindBufferMemory(device->Handle, internalState->handle, internalState->memory, 0);

    if (data != nullptr)
    {
        internalState->Upload_data_to_buffer(buffer->ByteSize(), data);
    }

    return buffer;
}

void VulkanBuffer::SetData(void* data, const u64& size, const u64& offset)
{
    vkMapMemory(device->Handle, memory, offset, size, 0, &mapdata);
    memcpy(mapdata, data, size);
    vkUnmapMemory(device->Handle, memory);
}

void VulkanBuffer::Upload_data_to_buffer(const u64& size, void* data)
{
    VulkanBuffer       stagingBuffer(device);
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

    if (vkCreateBuffer(device->Handle, &stagingBufferInfo, nullptr, &stagingBuffer.handle) != VK_SUCCESS)
    {
        SERROR("Failed to create staging buffer.");
        return;
    }

    stagingBuffer.Allocate_buffer_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(device->Handle, stagingBuffer.handle, stagingBuffer.memory, 0);

    stagingBuffer.SetData(data, size);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.commandPool        = device->resourcesCommandPool[device->GetFrameIndex()];
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

    vkCmdCopyBuffer(cmd, stagingBuffer.handle, handle, 1, &bufferCopyRegion);

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

    vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
}

void VulkanBuffer::Allocate_buffer_memory(VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->Handle, handle, &memoryRequirements);

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
