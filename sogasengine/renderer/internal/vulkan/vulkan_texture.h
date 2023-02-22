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

    static void TransitionLayout(const VulkanDevice*      device,
                                 const VulkanTexture*     InTexture,
                                 const TextureDescriptor* textureDescriptor,
                                 VkImageLayout            srcLayout,
                                 VkImageLayout            dstLayout);

    static void
    CopyBufferToImage(const VulkanDevice* device, VkBuffer buffer, VkImage image, const u32& width, const u32& height);

    void Release() override;
    void SetData(void* data, const TextureDescriptor* texture_descriptor);

    const VkImage     GetHandle() const { return handle; }
    const VkImageView GetImageView() const { return imageView; }
    const VkSampler   GetSampler();

    VkDescriptorImageInfo descriptorImageInfo;

  private:
    void*               mapdata   = nullptr;
    const VulkanDevice* device    = nullptr;
    VkImage             handle    = VK_NULL_HANDLE;
    VkImageView         imageView = VK_NULL_HANDLE;
    VkDeviceMemory      memory    = VK_NULL_HANDLE;
    VkSampler           sampler   = VK_NULL_HANDLE;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
