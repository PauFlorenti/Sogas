#pragma once

#include "device_texture.h"
#include "texture.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;
class VulkanBuffer;

struct VulkanTextureDescriptor
{
    u32                width{0};
    u32                height{0};
    VkFormat           texture_format;
    u32                texture_format_stride{0};
    VkImageAspectFlags texture_aspect{VK_IMAGE_ASPECT_NONE};
};

class VulkanTexture : public DeviceTexture
{
  public:
    explicit VulkanTexture(const VulkanDevice* InDevice = nullptr);
    VulkanTexture(const VulkanTexture&)                        = delete;
    VulkanTexture(VulkanTexture&&)                             = delete;
    const VulkanTexture& operator=(const VulkanTexture& other) = delete;
    ~VulkanTexture() { Release(); }

    static void Create(const VulkanDevice* device, Texture* texture, void* data);

    static inline VulkanTexture* ToInternal(const Texture* InTexture)
    {
        return static_cast<VulkanTexture*>(InTexture->internalState);
    }

    void Release() override;
    void SetData(void* data);

    const VkImage     GetHandle() const { return handle; }
    const VkImageView GetImageView() const { return imageView; }
    const VkSampler   GetSampler();

    VkDescriptorImageInfo descriptorImageInfo;

  private:
    void TransitionLayout(VkImageLayout srcLayout, VkImageLayout dstLayout);
    void CopyBufferToImage(const VulkanBuffer* buffer);
    void Allocate_and_bind_texture_memory(VkMemoryPropertyFlags memory_properties);

    void*                   mapdata   = nullptr;
    const VulkanDevice*     device    = nullptr;
    VkImage                 handle    = VK_NULL_HANDLE;
    VkImageView             imageView = VK_NULL_HANDLE;
    VkDeviceMemory          memory    = VK_NULL_HANDLE;
    VkSampler               sampler   = VK_NULL_HANDLE;
    VulkanTextureDescriptor texture_descriptor;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
