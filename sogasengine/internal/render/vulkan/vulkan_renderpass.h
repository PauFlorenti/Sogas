#pragma once

#include "vulkan_types.h"

namespace Sogas
{
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(VulkanRenderPass&) = delete;
        VulkanRenderPass(VulkanRenderPass&&) = delete;

    private:
        VkRenderPass renderpass;
        
    };

} // Sogas
