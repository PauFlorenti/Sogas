#pragma once

#include "device_swapchain.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
class Swapchain;
}
namespace Vk
{
class VulkanDevice;
class VulkanSwapchain : public Renderer::DeviceSwapchain
{
  public:
    explicit VulkanSwapchain(const VulkanDevice* InDevice = nullptr);
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)      = delete;
    ~VulkanSwapchain();

    static bool Create(const VulkanDevice* device, std::shared_ptr<Renderer::Swapchain> swapchain);

    static inline VulkanSwapchain* ToInternal(const std::shared_ptr<Renderer::Swapchain> InSwapchain)
    {
        return static_cast<VulkanSwapchain*>(InSwapchain->internalState);
    }

    void Destroy() override;

    VkSwapchainKHR     swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR       surface   = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    VkExtent2D         extent;

    u32                        imageIndex{0};
    std::vector<VkImage>       images;
    std::vector<VkImageView>   imageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderCompleteSemaphore  = VK_NULL_HANDLE;

  private:
    Texture             texture;
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Sogas
