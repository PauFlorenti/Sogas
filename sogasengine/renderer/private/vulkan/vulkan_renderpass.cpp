#include "vulkan/vulkan_attachment.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_texture.h"

namespace Sogas
{
namespace Vk
{
    void VulkanRenderPass::Create(const VulkanDevice* InDevice, const RenderPassDescriptor* desc, RenderPass* InRenderpass)
    {        
        auto internalState = std::make_shared<VulkanRenderPass>();

        InRenderpass->internalState = internalState;
        InRenderpass->descriptor    = *desc;

        std::vector<VkAttachmentReference> attachmentReferences;
        VkAttachmentReference depthAttachmentReference = {};

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::vector<VkAttachmentDescription> attachments(desc->attachments.size());
        std::vector<VkImageView> attachmentImageViews(desc->attachments.size());
        std::vector<VkAttachmentReference> references;
        u32 attachmentCount = 0;
        for (auto& att : desc->attachments)
        {
            auto attachmentInternalState = VulkanAttachment::ToInternal(att.attachmentFramebuffer);

            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.samples       = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.initialLayout = ConvertImageLayout(att.initialLayout);
            attachmentDescription.finalLayout   = ConvertImageLayout(att.finalLayout);
            attachmentDescription.format        = attachmentInternalState->GetFormat();
            attachmentDescription.loadOp        = ConvertLoadOperation(att.loadop);
            attachmentDescription.storeOp       = ConvertStoreOperation(att.storeop);

            attachmentImageViews.at(attachmentCount) = attachmentInternalState->GetImageView();

            if (att.type == Attachment::Type::RENDERTARGET)
            {
                VkAttachmentReference ref = {};
                ref.attachment  = attachmentCount;
                ref.layout      = ConvertImageLayout(att.subpassLayout);

                subpass.colorAttachmentCount++;

                references.push_back(ref);
            }
            else if (att.type == Attachment::Type::DEPTH_STENCIL)
            {
                depthAttachmentReference.attachment = attachmentCount;
                depthAttachmentReference.layout     = ConvertImageLayout(att.subpassLayout);
                subpass.pDepthStencilAttachment     = &depthAttachmentReference;
            }

            attachments.at(attachmentCount) = std::move(attachmentDescription);
            attachmentCount++;
        }

        subpass.pColorAttachments = references.data();

        VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        info.attachmentCount    = static_cast<u32>(attachments.size());
        info.pAttachments       = attachments.data();
        info.subpassCount       = 1;
        info.pSubpasses         = &subpass;
        
        if (vkCreateRenderPass(InDevice->Handle, &info, nullptr, &internalState->renderpass) != VK_SUCCESS) {
            SFATAL("Failed to create render pass.");
            SASSERT(false);
        }

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.attachmentCount = attachmentCount;
        framebufferInfo.renderPass      = internalState->renderpass;

        if (attachmentCount > 0)
        {
            framebufferInfo.pAttachments    = attachmentImageViews.data();
            framebufferInfo.width           = 640; //desc->attachments.at(0).texture->descriptor.width;
            framebufferInfo.height          = 480; //desc->attachments.at(0).texture->descriptor.height;
            framebufferInfo.layers          = 1;
        }

        if (vkCreateFramebuffer(InDevice->Handle, &framebufferInfo, nullptr, &internalState->framebuffer) != VK_SUCCESS) {
            SFATAL("Failed to create renderpass' framebuffer");
            SASSERT(false);
        }

        internalState->beginInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        internalState->beginInfo.framebuffer        = internalState->framebuffer;
        internalState->beginInfo.renderPass         = internalState->renderpass;
        internalState->beginInfo.renderArea.offset  = {0, 0};
        internalState->beginInfo.renderArea.extent  = {framebufferInfo.width, framebufferInfo.height};

        if (attachmentCount > 0)
        {
            internalState->beginInfo.clearValueCount    = attachmentCount;
            internalState->beginInfo.pClearValues       = internalState->clearColor;

            u32 i = 0;
            for (auto& att : InRenderpass->descriptor.attachments)
            {
                if (att.type == Attachment::Type::RENDERTARGET)
                {
                    internalState->clearColor[i].color.float32[0] = att.clear[0];
                    internalState->clearColor[i].color.float32[1] = att.clear[1];
                    internalState->clearColor[i].color.float32[2] = att.clear[2];
                    internalState->clearColor[i].color.float32[3] = att.clear[3];
                }
                if (att.type == Attachment::Type::DEPTH_STENCIL)
                {
                    internalState->clearColor[i].depthStencil.depth     = 1.0f;
                    internalState->clearColor[i].depthStencil.stencil   = 0;
                }
                i++;
            }
        }
    }
} // namespace Vk
} // namespace Sogas