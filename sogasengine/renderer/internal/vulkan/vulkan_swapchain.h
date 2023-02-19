#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
class VulkanSwapchain
{
  public:
    VulkanSwapchain()                       = default;
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)      = delete;
    ~VulkanSwapchain();

    static bool Create(const VulkanDevice*              device,
                       const SwapchainDescriptor*       descriptor,
                       std::shared_ptr<VulkanSwapchain> internalState);

    static inline std::shared_ptr<VulkanSwapchain> ToInternal(const std::shared_ptr<Swapchain> InSwapchain)
    {
        return std::static_pointer_cast<VulkanSwapchain>(InSwapchain->internalState);
    }

    void Release();

    VkSwapchainKHR     swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR       surface   = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    VkExtent2D         extent;

    u32                                       imageIndex{0};
    std::vector<VkImage>                      images;
    std::vector<VkImageView>                  imageViews;
    std::vector<VkFramebuffer>                framebuffers;

    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderCompleteSemaphore  = VK_NULL_HANDLE;

    Texture    texture;
    RenderPass renderpass;

  private:
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Sogas
