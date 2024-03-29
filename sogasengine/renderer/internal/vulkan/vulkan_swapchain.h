#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;
class VulkanRenderPass;
class VulkanSwapchain
{
  public:
    explicit VulkanSwapchain(VulkanDevice* InDevice = nullptr);
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)      = delete;
    ~VulkanSwapchain();

    static bool Create(VulkanDevice* device, std::shared_ptr<VulkanSwapchain> swapchain);

    void           CreateRenderPass(VulkanRenderPass* render_pass);
    void           Destroy();

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
    VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
