#pragma once

#include "device_swapchain.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
class Texture;
namespace Vk
{
class VulkanDevice;
class VulkanRenderPass;
class VulkanSwapchain : public Renderer::DeviceSwapchain
{
  public:
    explicit VulkanSwapchain(VulkanDevice* InDevice = nullptr);
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)      = delete;
    ~VulkanSwapchain();

    static bool Create(VulkanDevice* device, std::shared_ptr<VulkanSwapchain> swapchain);

    static inline VulkanSwapchain* ToInternal(const std::shared_ptr<Renderer::Swapchain> InSwapchain)
    {
        return static_cast<VulkanSwapchain*>(InSwapchain->internalState);
    }

    void           Destroy() override;
    const Texture* GetTexture() const
    {
        return texture;
    }

    VkSwapchainKHR     swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR       surface   = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    RenderPassOutput   output;

    u16 width  = 0;
    u16 height = 0;

    u32                        imageIndex{0};
    std::vector<VkImage>       images;
    std::vector<VkImageView>   imageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderCompleteSemaphore  = VK_NULL_HANDLE;

  private:
    void CreateRenderPass(VulkanRenderPass* render_pass);

    Texture*      texture;
    VkRenderPass  renderpass;
    VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
