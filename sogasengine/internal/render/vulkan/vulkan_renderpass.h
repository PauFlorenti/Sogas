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

        static inline std::shared_ptr<VulkanRenderPass> ToInternal(const RenderPass* InRenderpass) {
            return std::static_pointer_cast<VulkanRenderPass>(InRenderpass->internalState);
        }

        static void Create(const VulkanDevice* device, const RenderPassDescriptor* desc, RenderPass* renderpass);

        VkRenderPass            renderpass      = VK_NULL_HANDLE;
        VkFramebuffer           framebuffer     = VK_NULL_HANDLE;
        VkRenderPassBeginInfo   beginInfo       = {};
        VkClearValue            clearColor[8]; // Maximum of 8 attachments at the moment.
    };

} // Vk
} // Sogas
