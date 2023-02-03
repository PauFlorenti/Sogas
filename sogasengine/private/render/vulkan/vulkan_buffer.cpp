
#include "render/vulkan/vulkan_buffer.h"
#include "render/vulkan/vulkan_device.h"

namespace Sogas
{
    namespace Vk
    {
        std::unique_ptr<Renderer::Buffer> VulkanBuffer::Create(const VulkanDevice *device, Renderer::BufferDescriptor desc, void *data)
        {
            std::unique_ptr<Renderer::Buffer> buffer = std::make_unique<Renderer::Buffer>(desc);
            auto internal_state = std::make_shared<VulkanBuffer>();
            buffer->internal_state = internal_state;

            VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            buffer_info.size = buffer->getSizeInBytes();

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
                buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
                buffer_info.queueFamilyIndexCount = 2;
                buffer_info.pQueueFamilyIndices = families;
            }
            else
                buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device->Handle, &buffer_info, nullptr, &internal_state->handle) != VK_SUCCESS)
            {
                SERROR("Failed to create buffer.");
                buffer->internal_state.reset();
                return buffer;
            }

            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(device->Handle, internal_state->handle, &memoryRequirements);

            VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            allocateInfo.allocationSize = memoryRequirements.size;
            allocateInfo.memoryTypeIndex = device->FindMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

            if (vkAllocateMemory(device->Handle, &allocateInfo, nullptr, &internal_state->memory))
            {
                SERROR("Failed to allocate buffer memory.");
                buffer->internal_state.reset();
                return buffer;
            }

            vkBindBufferMemory(device->Handle, internal_state->handle, internal_state->memory, 0);

            if (data != nullptr)
            {
                VulkanBuffer stagingBuffer;
                VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};

                if (device->GraphicsFamily != device->PresentFamily)
                {
                    stagingBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
                    stagingBufferInfo.queueFamilyIndexCount = 2;
                    stagingBufferInfo.pQueueFamilyIndices = families;
                }
                else
                {
                    stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                }

                stagingBufferInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                stagingBufferInfo.size = buffer_info.size;

                if (vkCreateBuffer(device->Handle, &stagingBufferInfo, nullptr, &stagingBuffer.handle) != VK_SUCCESS)
                {
                    SERROR("Failed to create staging buffer.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                VkMemoryRequirements stagingMemoryRequirements;
                vkGetBufferMemoryRequirements(device->Handle, stagingBuffer.handle, &stagingMemoryRequirements);

                VkMemoryAllocateInfo stagingMemoryAllocationInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                stagingMemoryAllocationInfo.allocationSize = stagingMemoryRequirements.size;
                stagingMemoryAllocationInfo.memoryTypeIndex = device->FindMemoryType(stagingMemoryRequirements.memoryTypeBits,
                                                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                if (vkAllocateMemory(device->Handle, &stagingMemoryAllocationInfo, nullptr, &stagingBuffer.memory) != VK_SUCCESS)
                {
                    SERROR("Failed to allocate staging buffer memory.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                vkBindBufferMemory(device->Handle, stagingBuffer.handle, stagingBuffer.memory, 0);

                void *mapdata;
                vkMapMemory(device->Handle, stagingBuffer.memory, 0, buffer_info.size, 0, &mapdata);
                memcpy(mapdata, data, buffer_info.size);
                vkUnmapMemory(device->Handle, stagingBuffer.memory);

                VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                commandBufferAllocateInfo.commandBufferCount = 1;
                commandBufferAllocateInfo.commandPool = device->resourcesCommandPool[device->GetFrameIndex()];
                commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                VkCommandBuffer cmd;
                if (vkAllocateCommandBuffers(device->Handle, &commandBufferAllocateInfo, &cmd) != VK_SUCCESS)
                {
                    SERROR("Failed to allocate command buffer.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

                if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
                {
                    SERROR("Failed to begin recording the command buffer.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                VkBufferCopy bufferCopyRegion;
                bufferCopyRegion.size = buffer_info.size;
                bufferCopyRegion.srcOffset = 0;
                bufferCopyRegion.dstOffset = 0;

                vkCmdCopyBuffer(cmd, stagingBuffer.handle, internal_state->handle, 1, &bufferCopyRegion);

                if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
                {
                    SERROR("Failed to finish recording the copy command buffer.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &cmd;

                if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
                {
                    SERROR("Failed to submit copy buffer commands.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                if (vkQueueWaitIdle(device->GraphicsQueue) != VK_SUCCESS)
                {
                    SERROR("Failed waiting graphics queue to submit copy buffer commands.");
                    buffer->internal_state.reset();
                    return buffer;
                }

                vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
                vkDestroyBuffer(device->Handle, stagingBuffer.handle, nullptr);
                vkFreeMemory(device->Handle, stagingBuffer.memory, nullptr);
            }

            return buffer;
        }

    } // namespace Vk
} // namespace Sogas
