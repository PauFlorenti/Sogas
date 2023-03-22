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
    explicit VulkanTexture(const VulkanDevice* InDevice = nullptr);
    explicit VulkanTexture(const VulkanDevice* InDevice, const TextureDescriptor& InDescriptor);
    VulkanTexture(const VulkanTexture&)                        = delete;
    VulkanTexture(VulkanTexture&&)                             = delete;
    const VulkanTexture& operator=(const VulkanTexture& other) = delete;
    ~VulkanTexture()
    {
        Release();
    }

    static TextureHandle Create(VulkanDevice* device, const TextureDescriptor& InDescriptor);
    static void          Create(const VulkanDevice* device, Texture* texture, void* data);
    static std::shared_ptr<Texture>
    Create(const VulkanDevice* device, TextureDescriptor descriptor, void* data = nullptr);

    static inline VulkanTexture* ToInternal(const Texture* InTexture)
    {
        return static_cast<VulkanTexture*>(InTexture->internalState);
    }

    void Release() override;
    void SetData(void* data);

    const VkImage GetHandle() const
    {
        return texture;
    }
    const VkImageView GetImageView() const
    {
        return image_view;
    }
    const VkSampler GetSampler();

    VkDescriptorImageInfo descriptorImageInfo;

    TextureHandle           handle;
    VulkanTextureDescriptor descriptor;
    VkImage                 texture    = VK_NULL_HANDLE;
    VkImageView             image_view = VK_NULL_HANDLE;
    VkDeviceMemory          memory     = VK_NULL_HANDLE;
    VkSampler               sampler    = VK_NULL_HANDLE;
    VkImageLayout           image_layout;

    void Allocate_and_bind_texture_memory(VkMemoryPropertyFlags memory_properties);

  private:
    void TransitionLayout(VkImageLayout srcLayout, VkImageLayout dstLayout);
    void CopyBufferToImage(const VulkanBuffer* buffer);

    void*               mapdata = nullptr;
    const VulkanDevice* device  = nullptr;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
