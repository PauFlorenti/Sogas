#pragma once

#include "vulkan_types.h"

namespace Sogas
{
    namespace Vk
    {
        class VulkanDevice;
        class VulkanAttachment
        {
        public:

            ~VulkanAttachment() { Destroy(); }

            static inline std::shared_ptr<VulkanAttachment> ToInternal(const AttachmentFramebuffer *InAttachment)
            {
                return std::static_pointer_cast<VulkanAttachment>(InAttachment->internalState);
            }

            static void Create(const VulkanDevice *InDevice, AttachmentFramebuffer *InAttachment);

            const VkImage GetImage() const { return image; }
            const VkImageView GetImageView() const { return imageView; }
            const VkFormat GetFormat() const { return format; }

            void Destroy();

            VkDescriptorImageInfo imageInfo = {};

        private:
            VkDevice device = VK_NULL_HANDLE;
            VkImage image = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkFormat format;
        };
    } // namespace Vk
} // namespace Sogas
