
#include "vulkan/vulkan_texture.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_types.h"

// TODO Texture transition function abstraction
// TODO BufferCopyToImage function abstraction

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

VulkanTexture::VulkanTexture(const VulkanDevice* InDevice)
    : device(InDevice)
{}

VulkanTexture::VulkanTexture(const VulkanDevice* InDevice, const TextureDescriptor& InDescriptor)
    : device(InDevice)
{
    descriptor = InDescriptor;
}

void VulkanTexture::Create(const VulkanDevice* device, Texture* texture, void* data)
{
    SASSERT(device);
    SASSERT(texture);

    const auto& desc = texture->GetDescriptor();

    auto internalState        = new VulkanTexture(device);
    internalState->descriptor = desc;

    texture->internalState = internalState;

    // Create image
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    {
        imageInfo.format                = internalState->descriptor.texture_format;
        imageInfo.extent                = {desc.width, desc.height, 1};
        imageInfo.mipLevels             = 1;
        imageInfo.arrayLayers           = 1;
        imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = static_cast<u32>(device->queueFamilies.size());
        imageInfo.pQueueFamilyIndices   = device->queueFamilies.data();
        imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (desc.textureType)
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

        if (desc.usage == Usage::UPLOAD)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        if (desc.usage == Usage::READBACK)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (desc.bindPoint == BindPoint::SHADER_SAMPLE)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (desc.bindPoint == BindPoint::DEPTH_STENCIL)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (desc.bindPoint == BindPoint::RENDER_TARGET)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        VkDeviceSize imageSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth *
                                 imageInfo.arrayLayers * GetFormatStride(desc.format);

        if (vkCreateImage(device->Handle, &imageInfo, nullptr, &internalState->handle) != VK_SUCCESS)
        {
            SFATAL("Could not create image.");
            return;
        }
    }

    internalState->Allocate_and_bind_texture_memory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (data != nullptr)
    {
        internalState->SetData(data);
    }
    else
    {
        if (desc.bindPoint == BindPoint::DEPTH_STENCIL)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }
        if (desc.bindPoint == BindPoint::SHADER_SAMPLE)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        else if (desc.bindPoint == BindPoint::RENDER_TARGET)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }

    // Create image view
    {
        VkImageViewCreateInfo imageViewInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewInfo.image                           = internalState->handle;
        imageViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format                          = imageInfo.format;
        imageViewInfo.subresourceRange.aspectMask     = desc.bindPoint == BindPoint::DEPTH_STENCIL
                                                            ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                            : VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.layerCount     = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.levelCount     = 1;
        imageViewInfo.subresourceRange.baseMipLevel   = 0;

        if (vkCreateImageView(device->Handle, &imageViewInfo, nullptr, &internalState->imageView) != VK_SUCCESS)
        {
            SERROR("Failed to create image view!");
            return;
        }
    }
}

std::shared_ptr<Texture>
VulkanTexture::Create(const VulkanDevice* device, TextureDescriptor descriptor, void* data )
{
    SASSERT(device);

    auto texture           = std::make_shared<Texture>(std::move(descriptor));
    auto internalState     = new VulkanTexture(device, texture->GetDescriptor());
    auto desc              = internalState->descriptor;
    texture->internalState = internalState;

    // Create image
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    {
        imageInfo.format                = desc.texture_format;
        imageInfo.extent                = {desc.width, desc.height, 1};
        imageInfo.mipLevels             = 1;
        imageInfo.arrayLayers           = 1;
        imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = static_cast<u32>(device->queueFamilies.size());
        imageInfo.pQueueFamilyIndices   = device->queueFamilies.data();
        imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (texture->GetDescriptor().textureType)
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

        if (texture->GetDescriptor().usage == Usage::UPLOAD)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        if (texture->GetDescriptor().usage == Usage::READBACK)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if (texture->GetDescriptor().bindPoint == BindPoint::SHADER_SAMPLE)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (texture->GetDescriptor().bindPoint == BindPoint::DEPTH_STENCIL)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (texture->GetDescriptor().bindPoint == BindPoint::RENDER_TARGET)
        {
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        VkDeviceSize imageSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth *
                                 imageInfo.arrayLayers * GetFormatStride(texture->GetDescriptor().format);

        if (vkCreateImage(device->Handle, &imageInfo, nullptr, &internalState->handle) != VK_SUCCESS)
        {
            SFATAL("Could not create image.");
            return std::make_shared<Texture>();
        }
    }

    internalState->Allocate_and_bind_texture_memory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (data != nullptr)
    {
        internalState->SetData(data);
    }
    else
    {
        if (texture->GetDescriptor().bindPoint == BindPoint::DEPTH_STENCIL)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }
        if (texture->GetDescriptor().bindPoint == BindPoint::SHADER_SAMPLE)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        else if (texture->GetDescriptor().bindPoint == BindPoint::RENDER_TARGET)
        {
            internalState->TransitionLayout(imageInfo.initialLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }

    // Create image view
    {
        VkImageViewCreateInfo imageViewInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewInfo.image                           = internalState->handle;
        imageViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format                          = imageInfo.format;
        imageViewInfo.subresourceRange.aspectMask     = desc.texture_aspect;
        imageViewInfo.subresourceRange.layerCount     = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.levelCount     = 1;
        imageViewInfo.subresourceRange.baseMipLevel   = 0;

        if (vkCreateImageView(device->Handle, &imageViewInfo, nullptr, &internalState->imageView) != VK_SUCCESS)
        {
            SERROR("Failed to create image view!");
            return std::make_shared<Texture>();
        }
    }

    return texture;
}

void VulkanTexture::SetData(void* data)
{
    auto image_size = descriptor.width * descriptor.height * descriptor.texture_format_stride;

    VulkanBuffer       stagingBuffer(device);
    VkBufferCreateInfo bufferInfo    = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.queueFamilyIndexCount = static_cast<u32>(device->queueFamilies.size());
    bufferInfo.pQueueFamilyIndices   = device->queueFamilies.data();
    bufferInfo.size                  = image_size;

    if (vkCreateBuffer(device->Handle, &bufferInfo, nullptr, &stagingBuffer.handle) != VK_SUCCESS)
    {
        SERROR("Failed to create staging buffer for image texture.");
        return;
    }

    stagingBuffer.Allocate_buffer_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(device->Handle, stagingBuffer.handle, stagingBuffer.memory, 0);

    stagingBuffer.SetData(data, image_size);

    TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(&stagingBuffer);
    TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

const VkSampler VulkanTexture::GetSampler()
{
    if (sampler != VK_NULL_HANDLE)
    {
        return sampler;
    }

    VkSamplerCreateInfo samplerInfo     = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable        = VK_FALSE; // TODO check gpu properties and enable it!
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_TRUE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;

    if (vkCreateSampler(device->Handle, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        SFATAL("Failed to create sampler.");
    }
    return sampler;
}

void VulkanTexture::Release()
{
    {
        vkFreeMemory(device->Handle, memory, nullptr);
        vkDestroyImageView(device->Handle, imageView, nullptr);
        vkDestroyImage(device->Handle, handle, nullptr);
        vkDestroySampler(device->Handle, sampler, nullptr);
    }

    VkDevice       device    = VK_NULL_HANDLE;
    VkImage        handle    = VK_NULL_HANDLE;
    VkImageView    imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory    = VK_NULL_HANDLE;
    VkSampler      sampler   = VK_NULL_HANDLE;
}

void VulkanTexture::TransitionLayout(VkImageLayout srcLayout, VkImageLayout dstLayout)
{
    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount          = 1;
    cmdAllocInfo.commandPool                 = device->resourcesCommandPool[device->GetFrameIndex()];
    cmdAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(device->Handle, &cmdAllocInfo, &cmd) != VK_SUCCESS)
    {
        SERROR("Failed to allcoate command buffer.");
        return;
    }

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS)
    {
        VkImageMemoryBarrier barrier            = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.image                           = handle;
        barrier.oldLayout                       = srcLayout;
        barrier.newLayout                       = dstLayout;
        barrier.srcAccessMask                   = 0;
        barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = descriptor.texture_aspect;
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
        else if (srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                 dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
        else
        {
            SERROR("Unsupported layout transition.");
            return;
        }

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &cmd;

        if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            SERROR("Failed to submit graphics queue commands.");
            return;
        }

        vkQueueWaitIdle(device->GraphicsQueue);
    }
    vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
}

void VulkanTexture::CopyBufferToImage(const VulkanBuffer* buffer)
{
    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount          = 1;
    cmdAllocInfo.commandPool                 = device->resourcesCommandPool[device->GetFrameIndex()];
    cmdAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(device->Handle, &cmdAllocInfo, &cmd) != VK_SUCCESS)
    {
        SERROR("Failed to allcoate command buffer.");
        return;
    }

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS)
    {
        VkBufferImageCopy region{};
        region.bufferOffset                    = 0;
        region.bufferImageHeight               = 0;
        region.bufferRowLength                 = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {descriptor.width, descriptor.height, 1};

        vkCmdCopyBufferToImage(cmd, buffer->handle, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &cmd;

        if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            SERROR("Failed to submit graphics queue commands.");
            return;
        }

        vkQueueWaitIdle(device->GraphicsQueue);
    }
    vkFreeCommandBuffers(device->Handle, device->resourcesCommandPool[device->GetFrameIndex()], 1, &cmd);
}

void VulkanTexture::Allocate_and_bind_texture_memory(VkMemoryPropertyFlags memory_properties)
{
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device->Handle, handle, &memoryRequirements);
    {
        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize       = memoryRequirements.size;
        allocInfo.memoryTypeIndex      = device->FindMemoryType(memoryRequirements.memoryTypeBits, memory_properties);

        vkAllocateMemory(device->Handle, &allocInfo, nullptr, &memory);
        vkBindImageMemory(device->Handle, handle, memory, 0);
    }
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
