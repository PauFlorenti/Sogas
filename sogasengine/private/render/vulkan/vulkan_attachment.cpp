#include "render/vulkan/vulkan_attachment.h"
#include "render/vulkan/vulkan_device.h"

namespace Sogas
{
namespace Vk
{
    void VulkanAttachment::Create(const VulkanDevice* InDevice, AttachmentFramebuffer* InAttachment)
    {
        std::shared_ptr<VulkanAttachment> internalState = std::make_shared<VulkanAttachment>();
        InAttachment->internalState = internalState;

        internalState->format = ConvertFormat(InAttachment->format);

        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;
        VkImageUsageFlagBits imageUsage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        if (InAttachment->usage == BindPoint::RENDER_TARGET)
        {
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (InAttachment->usage == BindPoint::DEPTH_STENCIL)
        {
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.extent = {640, 480, 1}; // TODO pass current window extent.
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.format = internalState->format;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.usage = imageUsage | VK_IMAGE_USAGE_SAMPLED_BIT;

        if (vkCreateImage(InDevice->Handle, &imageInfo, nullptr, &internalState->image) != VK_SUCCESS) {
            SERROR("Failed to create image for attachment framebuffer.");
            SASSERT(false);
        }

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(InDevice->Handle, internalState->image, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = InDevice->FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        if (vkAllocateMemory(InDevice->Handle, &allocInfo, nullptr, &internalState->memory) != VK_SUCCESS) {
            SERROR("Failed to allocate memory for image attachment framebuffer");
            SASSERT(false);
        }

        if (vkBindImageMemory(InDevice->Handle, internalState->image, internalState->memory, 0) != VK_SUCCESS) {
            SERROR("Failed to bind image memory for image framebuffer");
            SASSERT(false);
        }

        VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewInfo.format = internalState->format;
        imageViewInfo.image = internalState->image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.subresourceRange.aspectMask = aspectMask;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseMipLevel = 0;

        if (vkCreateImageView(InDevice->Handle, &imageViewInfo, nullptr, &internalState->imageView) != VK_SUCCESS) {
            SERROR("Failed to create image view for image attachment framebuffer");
            return;
        }

    }
} // namespace Vk
} // namespace Sogas 