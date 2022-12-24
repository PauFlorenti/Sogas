
#include "render/vulkan/vulkan_buffer.h"
#include "render/vulkan/vulkan_device.h"

namespace Sogas
{
namespace Vk
{
    void VulkanBuffer::Create(
        const VulkanDevice& device, 
        const GPUBufferDescriptor* desc, 
        void* data, 
        GPUBuffer* buffer)
    {
        auto internalState = std::make_shared<VulkanBuffer>();
        buffer->internalState = internalState; 
        buffer->resourceType = GPUResource::ResourceType::BUFFER;
        buffer->mapdata = nullptr;
        buffer->descriptor = *desc;

        VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = buffer->descriptor.size;

        // Binding point
        if (buffer->descriptor.bindPoint == BindPoint::VERTEX)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (buffer->descriptor.bindPoint == BindPoint::INDEX)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (buffer->descriptor.bindPoint == BindPoint::UNIFORM)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        // Buffer usage
        if (buffer->descriptor.usage == Usage::UPLOAD)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (buffer->descriptor.usage == Usage::READBACK)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        u32 families[2] = {device.GraphicsFamily, device.PresentFamily};
        if (device.GraphicsFamily != device.PresentFamily)
        {
            bufferCreateInfo.sharingMode            = VK_SHARING_MODE_CONCURRENT;
            bufferCreateInfo.queueFamilyIndexCount  = 2;
            bufferCreateInfo.pQueueFamilyIndices    = families;
        }
        else
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device.Handle, &bufferCreateInfo, nullptr, &internalState->handle) != VK_SUCCESS)
        {
            SERROR("Failed to create buffer.");
            return;
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device.Handle, internalState->handle, &memoryRequirements);

        VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocateInfo.allocationSize  = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = device.FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device.Handle, &allocateInfo, nullptr, &internalState->memory))
        {
            SERROR("Failed to allocate buffer memory.");
            return;
        }

        vkBindBufferMemory(device.Handle, internalState->handle, internalState->memory, 0);

        // If initial data, upload to the buffer
        if (data != nullptr)
        {
            VulkanBuffer stagingBuffer;
            VkBufferCreateInfo stagingBufferInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            
            if (device.GraphicsFamily != device.PresentFamily)
            {
                stagingBufferInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
                stagingBufferInfo.queueFamilyIndexCount = 2;
                stagingBufferInfo.pQueueFamilyIndices   = families;
            }
            else {
                stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            stagingBufferInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            stagingBufferInfo.size  = buffer->descriptor.size;

            if (vkCreateBuffer(device.Handle, &stagingBufferInfo, nullptr, &stagingBuffer.handle) != VK_SUCCESS)
            {
                SERROR("Failed to create staging buffer.");
                return;
            }

            VkMemoryRequirements stagingMemoryRequirements;
            vkGetBufferMemoryRequirements(device.Handle, stagingBuffer.handle, &stagingMemoryRequirements);

            VkMemoryAllocateInfo stagingMemoryAllocationInfo {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            stagingMemoryAllocationInfo.allocationSize  = stagingMemoryRequirements.size;
            stagingMemoryAllocationInfo.memoryTypeIndex = device.FindMemoryType(stagingMemoryRequirements.memoryTypeBits, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (vkAllocateMemory(device.Handle, &stagingMemoryAllocationInfo, nullptr, &stagingBuffer.memory) != VK_SUCCESS)
            {
                SERROR("Failed to allocate staging buffer memory.");
                return;
            }

            vkBindBufferMemory(device.Handle, stagingBuffer.handle, stagingBuffer.memory, 0);

            vkMapMemory(device.Handle, stagingBuffer.memory, 0, buffer->descriptor.size, 0, &buffer->mapdata);
            memcpy(buffer->mapdata, data, buffer->descriptor.size);
            vkUnmapMemory(device.Handle, stagingBuffer.memory);

            VkCommandBufferAllocateInfo commandBufferAllocateInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            commandBufferAllocateInfo.commandBufferCount    = 1;
            commandBufferAllocateInfo.commandPool           = device.CommandPool;
            commandBufferAllocateInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(device.Handle, &commandBufferAllocateInfo, &cmd) != VK_SUCCESS)
            {
                SERROR("Failed to allocate command buffer.");
                return;
            }

            VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

            if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
            {
                SERROR("Failed to begin recording the command buffer.");
                return;
            }

            VkBufferCopy bufferCopyRegion;
            bufferCopyRegion.size = buffer->descriptor.size;
            bufferCopyRegion.srcOffset = 0;
            bufferCopyRegion.dstOffset = 0;

            vkCmdCopyBuffer(cmd, stagingBuffer.handle, internalState->handle, 1, &bufferCopyRegion);

            if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
                SERROR("Failed to finish recording the copy command buffer.");
                return;
            }

            VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &cmd;

            if (vkQueueSubmit(device.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            {
                SERROR("Failed to submit copy buffer commands.");
                return;
            }

            if (vkQueueWaitIdle(device.GraphicsQueue) != VK_SUCCESS)
            {
                SERROR("Failed waiting graphics queue to submit copy buffer commands.");
                return;
            }

            vkFreeCommandBuffers(device.Handle, device.CommandPool, 1, &cmd);
            vkDestroyBuffer(device.Handle, stagingBuffer.handle, nullptr);
            vkFreeMemory(device.Handle, stagingBuffer.memory, nullptr);
        }
    }

} // Vk
} // Sogas
