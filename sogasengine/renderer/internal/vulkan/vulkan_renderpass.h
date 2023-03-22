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

    static RenderPassHandle Create(VulkanDevice* InDevice, const RenderPassDescriptor& InDescriptor);
    static VkRenderPass     CreateRenderPass(const VulkanDevice* InDevice, const RenderPassOutput& InOutput, std::string InName);

    void Destroy() override;

    VkRenderPass     renderpass  = VK_NULL_HANDLE;
    VkFramebuffer    framebuffer = VK_NULL_HANDLE;
    RenderPassType   type;
    RenderPassOutput output;

    TextureHandle output_texture[MAX_IMAGE_OUTPUTS];
    TextureHandle depth_texture;

    f32 scale_x;
    f32 scale_y;
    u16 width      = 0;
    u16 heigh      = 0;
    u16 dispatch_x = 0;
    u16 dispatch_y = 0;
    u16 dispatch_z = 0;

    u8 resize               = 0;
    u8 render_targets_count = 0;

    std::string name;

    VkRenderPassBeginInfo beginInfo = {};
    VkClearValue          clearColor[8]; // Maximum of 8 attachments at the moment.

  private:
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
