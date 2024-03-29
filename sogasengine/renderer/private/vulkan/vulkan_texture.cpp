
#include "vulkan/vulkan_texture.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_commandbuffer.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
VkImageType ConverTextureType(TextureDescriptor::TextureType InType)
{
    switch (InType)
    {
        case TextureDescriptor::TextureType::TEXTURE_TYPE_1D:
            return VK_IMAGE_TYPE_1D;
            break;
        default:
        case TextureDescriptor::TextureType::TEXTURE_TYPE_2D:
            return VK_IMAGE_TYPE_2D;
            break;
        case TextureDescriptor::TextureType::TEXTURE_TYPE_3D:
            return VK_IMAGE_TYPE_3D;
            break;
    }
}

VkImageViewType ConvertTextureTypeToImageViewType(TextureDescriptor::TextureType InType)
{
    switch (InType)
    {
        case TextureDescriptor::TextureType::TEXTURE_TYPE_1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        default:
        case TextureDescriptor::TextureType::TEXTURE_TYPE_2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case TextureDescriptor::TextureType::TEXTURE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
    }
}
namespace Vk
{

static void CreateTexture(VkDevice InDevice, const TextureDescriptor& InDescriptor, TextureHandle InHandle, VulkanTexture* OutTexture)
{
    OutTexture->descriptor = InDescriptor;
    OutTexture->handle     = InHandle;

    const bool bIsRenderTarget = (InDescriptor.flags & TextureFlagsMask::RENDER_TARGET) == TextureFlagsMask::RENDER_TARGET;
    const bool bIsComputeUsed  = (InDescriptor.flags & TextureFlagsMask::COMPUTE) == TextureFlagsMask::COMPUTE;

    VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.format            = OutTexture->descriptor.format;
    image_info.flags             = 0;
    image_info.imageType         = OutTexture->descriptor.type;
    image_info.extent            = {InDescriptor.width, InDescriptor.height, InDescriptor.depth};
    image_info.mipLevels         = OutTexture->descriptor.mipmaps;
    image_info.arrayLayers       = 1;
    image_info.samples           = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling            = VK_IMAGE_TILING_OPTIMAL;

    // Default to always redeable from shader.
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.usage |= bIsComputeUsed ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

    if (HasDepthOrStencil(InDescriptor.format))
    {
        image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_info.usage |= bIsRenderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
    }

    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkcheck(vkCreateImage(InDevice, &image_info, nullptr, &OutTexture->texture));

    OutTexture->Allocate_and_bind_texture_memory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // TODO set name

    VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.image                 = OutTexture->texture;
    info.viewType              = ConvertTextureTypeToImageViewType(InDescriptor.type);
    info.format                = image_info.format;

    if (HasDepthOrStencil(InDescriptor.format))
    {
        info.subresourceRange.aspectMask = HasDepth(InDescriptor.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
    }
    else
    {
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;

    vkcheck(vkCreateImageView(InDevice, &info, nullptr, &OutTexture->image_view));

    OutTexture->image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

TextureHandle VulkanTexture::Create(VulkanDevice* InDevice, const TextureDescriptor& InDescriptor)
{
    TextureHandle handle = {InDevice->textures.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanTexture* texture = static_cast<VulkanTexture*>(InDevice->textures.AccessResource(handle.index));
    texture->device        = InDevice;

    CreateTexture(InDevice->Handle, InDescriptor, handle, texture);

    if (InDescriptor.data)
    {
        // Staging buffer
        u32          image_size = InDescriptor.width * InDescriptor.height * texture->descriptor.format_stride;
        VulkanBuffer staging_buffer;
        staging_buffer.device = InDevice;

        VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        buffer_info.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.size               = image_size;

        vkcheck(vkCreateBuffer(InDevice->Handle, &buffer_info, nullptr, &staging_buffer.buffer));

        staging_buffer.Allocate_buffer_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkcheck(vkBindBufferMemory(InDevice->Handle, staging_buffer.buffer, staging_buffer.memory, 0));

        // Copy data
        void* mapdata;
        vkMapMemory(InDevice->Handle, staging_buffer.memory, 0, image_size, 0, &mapdata);
        memcpy(mapdata, InDescriptor.data, image_size);
        vkUnmapMemory(InDevice->Handle, staging_buffer.memory);

        auto                     command_buffer = static_cast<VulkanCommandBuffer*>(InDevice->GetInstantCommandBuffer());
        VkCommandBufferBeginInfo begin_info     = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin_info.flags                        = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer->command_buffer, &begin_info);

        VkBufferImageCopy region               = {};
        region.bufferOffset                    = 0;
        region.bufferImageHeight               = 0;
        region.bufferRowLength                 = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {texture->descriptor.width, texture->descriptor.height, 1};

        bool is_depth = HasDepth(texture->descriptor.generic_format);
        TransitionLayout(command_buffer, texture->texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, is_depth);
        vkCmdCopyBufferToImage(command_buffer->command_buffer, staging_buffer.buffer, texture->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        TransitionLayout(command_buffer, texture->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, is_depth);

        vkEndCommandBuffer(command_buffer->command_buffer);

        VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &command_buffer->command_buffer;

        vkQueueSubmit(InDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(InDevice->GetGraphicsQueue());

        vkResetCommandBuffer(command_buffer->command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        vkDestroyBuffer(InDevice->Handle, staging_buffer.buffer, nullptr);
        vkFreeMemory(InDevice->Handle, staging_buffer.memory, nullptr);

        texture->image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    return handle;
}

void VulkanTexture::Release()
{
    {
        vkFreeMemory(device->Handle, memory, nullptr);
        vkDestroyImageView(device->Handle, image_view, nullptr);
        vkDestroyImage(device->Handle, texture, nullptr);
    }

    VkDevice       device    = VK_NULL_HANDLE;
    VkImage        texture   = VK_NULL_HANDLE;
    VkImageView    imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory    = VK_NULL_HANDLE;
    VkSampler      sampler   = VK_NULL_HANDLE;
}

void VulkanTexture::TransitionLayout(
  VulkanCommandBuffer* command_buffer,
  VkImage              image,
  VkImageLayout        source_layout,
  VkImageLayout        destination_layout,
  bool                 is_depth)
{
    VkImageMemoryBarrier barrier            = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image                           = image;
    barrier.oldLayout                       = source_layout;
    barrier.newLayout                       = destination_layout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.layerCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseMipLevel   = 0;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if (source_layout == VK_IMAGE_LAYOUT_UNDEFINED && destination_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (source_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             destination_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (source_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             destination_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (source_layout == VK_IMAGE_LAYOUT_UNDEFINED && destination_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    else
    {
        SERROR("Unsupported layout transition.");
        //return;
    }

    vkCmdPipelineBarrier(command_buffer->command_buffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanTexture::Allocate_and_bind_texture_memory(VkMemoryPropertyFlags memory_properties)
{
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device->Handle, texture, &memoryRequirements);
    {
        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize       = memoryRequirements.size;
        allocInfo.memoryTypeIndex      = device->FindMemoryType(memoryRequirements.memoryTypeBits, memory_properties);

        vkAllocateMemory(device->Handle, &allocInfo, nullptr, &memory);
        vkBindImageMemory(device->Handle, texture, memory, 0);
    }
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
