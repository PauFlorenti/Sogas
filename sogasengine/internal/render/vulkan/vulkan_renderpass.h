#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Vk
{
    class VulkanDevice;
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass() = default;
        VulkanRenderPass(const VulkanRenderPass&) = delete;
        VulkanRenderPass(VulkanRenderPass&&) = delete;
        const VulkanRenderPass& operator=(const VulkanRenderPass& other) = delete;

        static inline std::shared_ptr<VulkanRenderPass> ToInternalState(const RenderPass* renderpass);
        static void Create(const VulkanDevice* device, const RenderPassDescriptor* desc, RenderPass* renderpass);

        VkRenderPass renderpass   = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkClearValue clearColor;
    };

} // Vk
} // Sogas
