#include "vulkan/vulkan_attachment.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

void VulkanAttachment::Create(const VulkanDevice* InDevice, AttachmentFramebuffer* InAttachment)
{
    std::shared_ptr<VulkanAttachment> internalState = std::make_shared<VulkanAttachment>();
    InAttachment->internalState                     = internalState;
    internalState->device                           = InDevice->Handle;
    internalState->format                           = ConvertFormat(InAttachment->format);

    VkImageAspectFlags   aspectMask = 0;
    VkImageLayout        imageLayout;
    VkImageUsageFlagBits imageUsage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
    if (InAttachment->usage == BindPoint::RENDER_TARGET)
    {
        aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
        imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (InAttachment->usage == BindPoint::DEPTH_STENCIL)
    {
        aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        imageUsage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.arrayLayers       = 1;
    imageInfo.mipLevels         = 1;
    imageInfo.extent            = {640, 480, 1}; // TODO pass current window extent.
    imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.format            = internalState->format;
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage             = imageUsage | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (vkCreateImage(InDevice->Handle, &imageInfo, nullptr, &internalState->image) != VK_SUCCESS)
    {
        SERROR("Failed to create image for attachment framebuffer.");
        SASSERT(false);
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(InDevice->Handle, internalState->image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize       = memoryRequirements.size;
    allocInfo.memoryTypeIndex =
        InDevice->FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(InDevice->Handle, &allocInfo, nullptr, &internalState->memory) != VK_SUCCESS)
    {
        SERROR("Failed to allocate memory for image attachment framebuffer");
        SASSERT(false);
    }

    if (vkBindImageMemory(InDevice->Handle, internalState->image, internalState->memory, 0) != VK_SUCCESS)
    {
        SERROR("Failed to bind image memory for image framebuffer");
        SASSERT(false);
    }

    VkImageViewCreateInfo imageViewInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewInfo.format                          = internalState->format;
    imageViewInfo.image                           = internalState->image;
    imageViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.subresourceRange.aspectMask     = aspectMask;
    imageViewInfo.subresourceRange.layerCount     = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.levelCount     = 1;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;

    if (vkCreateImageView(InDevice->Handle, &imageViewInfo, nullptr, &internalState->imageView) != VK_SUCCESS)
    {
        SERROR("Failed to create image view for image attachment framebuffer");
        return;
    }

    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount          = 1;
    cmdAllocInfo.commandPool                 = InDevice->resourcesCommandPool[InDevice->GetFrameIndex()];
    cmdAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(InDevice->Handle, &cmdAllocInfo, &cmd) != VK_SUCCESS)
    {
        SERROR("Failed to allcoate command buffer.");
        return;
    }

    if (InAttachment->usage == BindPoint::DEPTH_STENCIL)
    {
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS)
        {
            VkImageMemoryBarrier barrier            = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.image                           = internalState->image;
            barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask                   = 0;
            barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask     = aspectMask;
            barrier.subresourceRange.layerCount     = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.levelCount     = 1;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

            vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &cmd;

            if (vkQueueSubmit(InDevice->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            {
                SERROR("Failed to submit graphics queue commands.");
                return;
            }

            vkQueueWaitIdle(InDevice->GraphicsQueue);
        }
        vkFreeCommandBuffers(InDevice->Handle, InDevice->resourcesCommandPool[InDevice->GetFrameIndex()], 1, &cmd);
    }
}

const VkSampler VulkanAttachment::GetSampler()
{
    if (sampler != VK_NULL_HANDLE)
    {
        return sampler;
    }

    VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter           = VK_FILTER_NEAREST;
    samplerInfo.minFilter           = VK_FILTER_NEAREST;
    samplerInfo.borderColor         = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias          = 0.0f;
    samplerInfo.maxAnisotropy       = 1.0f;
    samplerInfo.minLod              = 0.0f;
    samplerInfo.maxLod              = 1.0f;

    vkCreateSampler(device, &samplerInfo, nullptr, &sampler);

    return sampler;
}

void VulkanAttachment::Destroy()
{
    {
        vkFreeMemory(device, memory, nullptr);
        vkDestroyImageView(device, imageView, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkDestroySampler(device, sampler, nullptr);
    }

    device    = VK_NULL_HANDLE;
    image     = VK_NULL_HANDLE;
    imageView = VK_NULL_HANDLE;
    memory    = VK_NULL_HANDLE;
    sampler   = VK_NULL_HANDLE;
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas