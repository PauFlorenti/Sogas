#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass() = default;
        VulkanRenderPass(VulkanRenderPass&) = delete;
        VulkanRenderPass(VulkanRenderPass&&) = delete;

        VkRenderPass renderpass   = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkClearValue clearColor;
    };

} // Vk
} // Sogas
