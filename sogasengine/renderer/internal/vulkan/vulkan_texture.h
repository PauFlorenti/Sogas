#pragma once

#include "device_texture.h"
#include "texture.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
VkImageType ConverTextureType(TextureDescriptor::TextureType InType);
namespace Vk
{
class VulkanDevice;
class VulkanBuffer;
class VulkanSampler;
class VulkanCommandBuffer;

void TransitionImageLayout(const VulkanDevice* device, VkCommandBuffer command_buffer, VkImage& image, VkImageLayout source_layout, VkImageLayout destination_layout, bool is_depth);

struct VulkanTextureDescriptor
{
    void operator=(const TextureDescriptor& other)
    {
        width          = other.width;
        height         = other.height;
        depth          = other.depth;
        mipmaps        = other.mipmaps;
        generic_format = other.format;
        format         = ConvertFormat(other.format);
        format_stride  = GetFormatStride(other.format);
        type           = ConverTextureType(other.type);
        aspect         = HasDepthOrStencil(other.format) ? (HasDepth(other.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) : VK_IMAGE_ASPECT_COLOR_BIT;
    };

    u16                width{1};
    u16                height{1};
    u16                depth{1};
    u8                 mipmaps{0};
    u32                format_stride{0};
    Format             generic_format;
    VkFormat           format;
    VkImageType        type{VK_IMAGE_TYPE_MAX_ENUM};
    VkImageAspectFlags aspect;
};

class VulkanTexture : public DeviceTexture
{
  public:
    explicit VulkanTexture(VulkanDevice* InDevice = nullptr);
    explicit VulkanTexture(VulkanDevice* InDevice, const TextureDescriptor& InDescriptor);
    VulkanTexture(const VulkanTexture&)                        = delete;
    VulkanTexture(VulkanTexture&&)                             = delete;
    const VulkanTexture& operator=(const VulkanTexture& other) = delete;
    ~VulkanTexture()
    {
        Release();
    }

    static TextureHandle Create(VulkanDevice* device, const TextureDescriptor& InDescriptor);
    static void TransitionLayout(
      VulkanCommandBuffer* cmd, 
      VkImage image, 
      VkImageLayout srcLayout, 
      VkImageLayout dstLayout,
      bool is_depth = false);

    void Release() override;
    void SetData(void* data);

    VkDescriptorImageInfo descriptorImageInfo;

    VkImage        texture    = VK_NULL_HANDLE;
    VkImageView    image_view = VK_NULL_HANDLE;
    VkDeviceMemory memory     = VK_NULL_HANDLE;
    VkImageLayout  image_layout;

    VulkanTextureDescriptor descriptor;
    TextureHandle           handle;
    VulkanSampler*          sampler = nullptr;
    void*                   mapdata = nullptr;

    void Allocate_and_bind_texture_memory(VkMemoryPropertyFlags memory_properties);

  private:
    void CopyBufferToImage(const VulkanBuffer* buffer);

    VulkanDevice* device = nullptr;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
