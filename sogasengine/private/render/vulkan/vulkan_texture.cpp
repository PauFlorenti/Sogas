
#include "render/vulkan/vulkan_buffer.h"
#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_texture.h"

// TODO Texture transition function abstraction
// TODO BufferCopyToImage function abstraction

namespace Sogas
{
namespace Vk
{
    void VulkanTexture::Create(
        const VulkanDevice* device,
        const TextureDescriptor* desc,
        void* data,
        Texture* texture)
    {
        SASSERT(device);
        SASSERT(desc);
        SASSERT(texture && texture->IsTexture());

        auto internalState = std::make_shared<VulkanTexture>();
        texture->descriptor = *desc;
        texture->resourceType = GPUResource::ResourceType::TEXTURE;
        texture->internalState = internalState;
        
        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.format = ConvertFormat(desc->format);
        imageInfo.extent = {desc->width, desc->height, 1};
        imageInfo.mipLevels = 1; // TODO pass info from descriptor
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = static_cast<u32>(device->queueFamilies.size());
        imageInfo.pQueueFamilyIndices = device->queueFamilies.data();
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (desc->textureType)
        {
        case TextureDescriptor::TextureType::TEXTURE_TYPE_1D:
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            break;
        default:
        case TextureDescriptor::TextureType::TEXTURE_TYPE_2D:
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            break;
        case TextureDescriptor::TextureType::TEXTURE_TYPE_3D:
            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            break;
        }

        if (desc->usage == Usage::UPLOAD)
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (desc->usage == Usage::READBACK)
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        //if (desc->bindPoint == BindPoint::)
        //    imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (desc->bindPoint == BindPoint::SHADER_SAMPLE)
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (desc->bindPoint == BindPoint::DEPTH_STENCIL)
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (desc->bindPoint == BindPoint::RENDER_TARGET)
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (desc->usage == Usage::READBACK)
        {
            VkDeviceSize imageSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth * imageInfo.arrayLayers * GetFormatStride(desc->format);

            VulkanTexture::TransitionLayout(
                device, 
                internalState->handle, 
                imageInfo.format, 
                imageInfo.initialLayout, 
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            if (vkCreateImage(device->Handle, &imageInfo, nullptr, &internalState->handle) != VK_SUCCESS) {
                SFATAL("Could not create image.");
                return;
            }

            VkMemoryRequirements memoryRequirements;
            vkGetImageMemoryRequirements(device->Handle, internalState->handle, &memoryRequirements);

            {   
                VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                allocInfo.allocationSize = memoryRequirements.size;
                allocInfo.memoryTypeIndex = device->FindMemoryType(memoryRequirements.memoryTypeBits, 0);

                vkAllocateMemory(device->Handle, &allocInfo, nullptr, &internalState->memory);

                vkBindImageMemory(device->Handle, internalState->handle, internalState->memory, 0);
            }
            VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

            imageViewInfo.image = internalState->handle;

            if (data != nullptr)
            {
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingMemory;
                VkBufferCreateInfo bufferInfo       = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                bufferInfo.usage                    = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                bufferInfo.queueFamilyIndexCount    = static_cast<u32>(device->queueFamilies.size());
                bufferInfo.pQueueFamilyIndices      = device->queueFamilies.data();
                bufferInfo.size                     = imageSize;
                
                if (vkCreateBuffer(device->Handle, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
                    SERROR("Failed to create staging buffer for image texture.");
                    return;
                }

                VkMemoryRequirements stagingMemoryRequirements;
                vkGetBufferMemoryRequirements(device->Handle, stagingBuffer, &stagingMemoryRequirements);

                {
                    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                    allocInfo.allocationSize = stagingMemoryRequirements.size;
                    allocInfo.memoryTypeIndex = device->FindMemoryType(stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                    if (vkAllocateMemory(device->Handle, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS) {
                        SERROR("Could not allocate memory for image staging buffer.");
                        return;
                    }

                    if (vkBindBufferMemory(device->Handle, stagingBuffer, stagingMemory, 0) != VK_SUCCESS) {
                        SERROR("Failed to bind buffer memory.");
                        return;
                    }
                }

                vkMapMemory(device->Handle, stagingMemory, 0, imageSize, 0, &texture->mapdata);
                memcpy(&texture->mapdata, &data, imageSize);
                vkUnmapMemory(device->Handle, stagingMemory);

                TransitionLayout(device, internalState->handle, imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                CopyBufferToImage(device, stagingBuffer, internalState->handle, imageInfo.extent.width, imageInfo.extent.height);
                TransitionLayout(device, internalState->handle, imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                vkDestroyBuffer(device->Handle, stagingBuffer, nullptr);
                vkFreeMemory(device->Handle, stagingMemory, nullptr);
            }
        }

        VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewInfo.image                             = internalState->handle;
        imageViewInfo.viewType                          = VK_IMAGE_VIEW_TYPE_2D; // TODO make it configurable
        imageViewInfo.format                            = imageInfo.format;
        imageViewInfo.subresourceRange.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.layerCount       = 1;
        imageViewInfo.subresourceRange.baseArrayLayer   = 0;
        imageViewInfo.subresourceRange.levelCount       = 1;
        imageViewInfo.subresourceRange.baseMipLevel     = 0;

        if (vkCreateImageView(device->Handle, &imageViewInfo, nullptr, &internalState->imageView) != VK_SUCCESS) {
            SERROR("Failed to create image view!");
            return;
        }
    }

    void VulkanTexture::TransitionLayout(
        const VulkanDevice* device,
        VkImage& image,
        VkFormat /*format*/,
        VkImageLayout srcLayout,
        VkImageLayout dstLayout)
    {
        VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cmdAllocInfo.commandBufferCount = 1;
        cmdAllocInfo.commandPool        = device->resourcesCommandPool[device->GetFrameIndex()];
        cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        VkCommandBuffer cmd;
        if (vkAllocateCommandBuffers(device->Handle, &cmdAllocInfo, &cmd) != VK_SUCCESS) {
            SERROR("Failed to allcoate command buffer.");
            return;
        }

        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS)
        {

            VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.image                           = image;
            barrier.oldLayout                       = srcLayout;
            barrier.newLayout                       = dstLayout;
            barrier.srcAccessMask                   = 0;
            barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.layerCount     = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.levelCount     = 1;
            barrier.subresourceRange.baseMipLevel   = 0;

            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;

            if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if(srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else {
                SERROR("Unsupported layout transition.");
                return;
            }

            vkCmdPipelineBarrier(
                cmd, 
                srcStage,
                dstStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            
            if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                SERROR("Failed to submit graphics queue commands.");
                return;
            }

            vkQueueWaitIdle(device->GraphicsQueue);
            vkEndCommandBuffer(cmd);
        }

        vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
    }

    void VulkanTexture::CopyBufferToImage(
        const VulkanDevice* device,
        VkBuffer buffer,
        VkImage image,
        const u32& width,
        const u32& height)
    {
        VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cmdAllocInfo.commandBufferCount = 1;
        cmdAllocInfo.commandPool        = device->resourcesCommandPool[device->GetFrameIndex()];
        cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        VkCommandBuffer cmd;
        if (vkAllocateCommandBuffers(device->Handle, &cmdAllocInfo, &cmd) != VK_SUCCESS) {
            SERROR("Failed to allcoate command buffer.");
            return;
        }

        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS)
        {
            VkBufferImageCopy region{};
            region.bufferOffset                     = 0;
            region.bufferImageHeight                = 0;
            region.bufferRowLength                  = 0;
            region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.baseArrayLayer  = 0;
            region.imageSubresource.mipLevel        = 0;
            region.imageSubresource.layerCount      = 1;
            region.imageOffset                      = {0, 0, 0};
            region.imageExtent                      = {width, height, 1};

            vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            
            if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                SERROR("Failed to submit graphics queue commands.");
                return;
            }

            vkQueueWaitIdle(device->GraphicsQueue);
            vkEndCommandBuffer(cmd);
        }
        vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
    }

} // Vk
} // Sogas
