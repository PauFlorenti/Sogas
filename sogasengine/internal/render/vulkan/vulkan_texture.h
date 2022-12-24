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
        VulkanTexture(VulkanTexture&) = delete;
        VulkanTexture(VulkanTexture&&) = delete;

        static void Create(
            const VulkanDevice* device,
            const TextureDescriptor* desc,
            void* data,
            Texture* texture);

        static void TransitionLayout(
            const VulkanDevice* device,
            VkImage& image,
            VkFormat format,
            VkImageLayout srcLayout,
            VkImageLayout dstLayout);

        static void CopyBufferToImage(
            const VulkanDevice* device,
            VkBuffer buffer,
            VkImage image,
            const u32& width, 
            const u32& height);
        
    private:
        VkImage     handle;
        VkImageView imageView;
        VkDeviceMemory memory;
    };
    
} // Vk
} // Sogas
