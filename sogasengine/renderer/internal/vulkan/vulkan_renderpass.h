#pragma once

#include "device_renderpass.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
class RenderPass;
struct RenderPassDescriptor;
namespace Vk
{
class VulkanDevice;
class VulkanRenderPass : public Renderer::DeviceRenderpass
{
  public:
    explicit VulkanRenderPass(const VulkanDevice* = nullptr);
    VulkanRenderPass(const VulkanRenderPass&)                        = delete;
    VulkanRenderPass(VulkanRenderPass&&)                             = delete;
    const VulkanRenderPass& operator=(const VulkanRenderPass& other) = delete;
    ~VulkanRenderPass();

    static inline VulkanRenderPass* ToInternal(const Renderer::RenderPass* InRenderpass)
    {
        return static_cast<VulkanRenderPass*>(InRenderpass->internalState);
    }

    static void Create(const VulkanDevice* device, Renderer::RenderPass* renderpass);

    void Destroy() override;

    VkRenderPass          renderpass  = VK_NULL_HANDLE;
    VkFramebuffer         framebuffer = VK_NULL_HANDLE;
    VkRenderPassBeginInfo beginInfo   = {};
    VkClearValue          clearColor[8]; // Maximum of 8 attachments at the moment.

  private:
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
