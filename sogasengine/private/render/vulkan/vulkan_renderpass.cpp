#include "render/vulkan/vulkan_renderpass.h"

namespace Sogas
{
namespace Vk
{
    std::shared_ptr<VulkanRenderPass> VulkanRenderPass::ToInternalState(const RenderPass* InRenderpass)
    {
        return std::static_pointer_cast<VulkanRenderPass>(InRenderpass->internalState);
    }

    void VulkanRenderPass::Create(const VulkanDevice* /*device*/, const RenderPassDescriptor* /*desc*/, RenderPass* /*renderpass*/)
    {
        /*
        auto internalState = std::make_shared<VulkanRenderPass>();

        renderpass->internalState = internalState;
        renderpass->descriptor = *desc;

        for (auto& att : desc->attachments)
        {
            
        }
        */
    }
} // Vk
} // Sogas
