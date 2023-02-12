#pragma once

#include "vulkan_types.h"
namespace Sogas
{
    namespace Vk
    {
        class VulkanDevice;
        class VulkanTexture
        {
        public:
            VulkanTexture() = default;
            VulkanTexture(const VulkanTexture &) = delete;
            VulkanTexture(VulkanTexture &&) = delete;
            const VulkanTexture &operator=(const VulkanTexture &other) = delete;
            ~VulkanTexture() { Release(); }

            static void Create(
                const VulkanDevice *device,
                const TextureDescriptor *desc,
                void *data,
                Texture *texture);

            static inline std::shared_ptr<VulkanTexture> ToInternal(const Texture *InTexture)
            {
                return std::static_pointer_cast<VulkanTexture>(InTexture->internalState);
            }

            static void TransitionLayout(
                const VulkanDevice *device,
                const Texture *InTexture,
                VkImageLayout srcLayout,
                VkImageLayout dstLayout);

            static void CopyBufferToImage(
                const VulkanDevice *device,
                VkBuffer buffer,
                VkImage image,
                const u32 &width,
                const u32 &height);

            void Release();

            const VkImage GetHandle() const { return handle; }
            const VkImageView GetImageView() const { return imageView; }
            const VkSampler GetSampler();

            VkDescriptorImageInfo descriptorImageInfo;

        private:
            VkDevice        device      = VK_NULL_HANDLE;
            VkImage         handle      = VK_NULL_HANDLE;
            VkImageView     imageView   = VK_NULL_HANDLE;
            VkDeviceMemory  memory      = VK_NULL_HANDLE;
            VkSampler       sampler     = VK_NULL_HANDLE;
        };
    } // namespace Vk
} // namespace Sogas
