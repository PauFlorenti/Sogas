#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_texture.h"
#include "vulkan/vulkan_swapchain.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

static RenderPassOutput FillRenderPassOutput(VulkanDevice* InDevice, const RenderPassDescriptor& InDescriptor)
{
    RenderPassOutput output;
    output.Reset();

    for (u32 i = 0; i < InDescriptor.RenderTargetsCount; ++i)
    {
        VulkanTexture* texture = static_cast<VulkanTexture*>(InDevice->textures.AccessResource(InDescriptor.OutputTextures[i].index));
        output.AddColor(texture->descriptor.generic_format);
    }

    if (InDescriptor.DepthStencilTexture.index != INVALID_ID)
    {
        VulkanTexture* texture = static_cast<VulkanTexture*>(InDevice->textures.AccessResource(InDescriptor.DepthStencilTexture.index));
        output.SetDepth(texture->descriptor.generic_format);
    }

    output.ColorOperation   = InDescriptor.ColorOperation;
    output.DepthOperation   = InDescriptor.DepthOperation;
    output.StencilOperation = InDescriptor.StencilOperation;

    return output;
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice* InDevice)
: device(InDevice)
{
    SASSERT(InDevice != nullptr);
}

VulkanRenderPass::~VulkanRenderPass()
{
    Destroy();
}

RenderPassHandle VulkanRenderPass::Create(VulkanDevice* InDevice, const RenderPassDescriptor& InDescriptor)
{
    RenderPassHandle handle = {InDevice->renderpasses.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanRenderPass* render_pass     = static_cast<VulkanRenderPass*>(InDevice->renderpasses.AccessResource(handle.index));
    render_pass->type                 = InDescriptor.Type;
    render_pass->render_targets_count = static_cast<u8>(InDescriptor.RenderTargetsCount);
    render_pass->dispatch_x           = 0;
    render_pass->dispatch_y           = 0;
    render_pass->dispatch_z           = 0;
    render_pass->name                 = InDescriptor.Name;
    render_pass->scale_x              = InDescriptor.ScaleX;
    render_pass->scale_y              = InDescriptor.ScaleY;
    render_pass->resize               = InDescriptor.Resize;

    for (u32 i = 0; i < InDescriptor.RenderTargetsCount; ++i)
    {
        VulkanTexture* texture = static_cast<VulkanTexture*>(InDevice->textures.AccessResource(InDescriptor.OutputTextures[i].index));

        render_pass->width = texture->descriptor.width;
        render_pass->heigh = texture->descriptor.height;

        render_pass->output_texture[i] = InDescriptor.OutputTextures[i];
    }

    render_pass->depth_texture = InDescriptor.DepthStencilTexture;

    switch (InDescriptor.Type)
    {
        case RenderPassType::SWAPCHAIN:
        {
            InDevice->swapchain->CreateRenderPass(render_pass);
            break;
        }
        case RenderPassType::COMPUTE:
        {
            break;
        }
        case RenderPassType::GEOMETRY:
        {
            render_pass->output     = FillRenderPassOutput(InDevice, InDescriptor);
            render_pass->renderpass = InDevice->GetVulkanRenderPass(render_pass->output, InDescriptor.Name);
            break;
        }
    }
    return handle;
}

VkRenderPass VulkanRenderPass::CreateRenderPass(const VulkanDevice* InDevice, const RenderPassOutput& InOutput, std::string InName)
{
    VkAttachmentDescription color_attachments[8]     = {};
    VkAttachmentReference   color_attachment_refs[8] = {};

    VkAttachmentLoadOp color_op, depth_op, stencil_op;
    VkImageLayout      color_initial_layout, depth_initial_layout;

    switch (InOutput.ColorOperation)
    {
        case RenderPassOperation::LOAD:
            color_op             = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOperation::CLEAR:
            color_op             = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        default:
            color_op             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
    }

    switch (InOutput.DepthOperation)
    {
        case RenderPassOperation::LOAD:
            depth_op             = VK_ATTACHMENT_LOAD_OP_LOAD;
            depth_initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOperation::CLEAR:
            depth_op             = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            depth_op             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
    }

    switch (InOutput.StencilOperation)
    {
        case RenderPassOperation::LOAD:
            stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RenderPassOperation::CLEAR:
            stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
    }

    u32 i = 0;
    for (; i < InOutput.ColorFormatCounts; ++i)
    {
        VkAttachmentDescription& color_attachment = color_attachments[i];
        color_attachment.format                  = ConvertFormat(InOutput.ColorFormats[i]);
        color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp                  = color_op;
        color_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp           = stencil_op;
        color_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout           = color_initial_layout;
        color_attachment.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference& color_attachment_reference = color_attachment_refs[i];
        color_attachment_reference.attachment             = i;
        color_attachment_reference.layout                 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentDescription depth_attachment{};
    VkAttachmentReference   depth_attachment_reference{};

    if (InOutput.DepthStencilFormat != Format::UNDEFINED)
    {
        depth_attachment.format         = ConvertFormat(InOutput.DepthStencilFormat);
        depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp         = depth_op;
        depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp  = stencil_op;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout  = depth_initial_layout;
        depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depth_attachment_reference.attachment = i;
        depth_attachment_reference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
    u32                     active_attachments = 0;
    for (; active_attachments < InOutput.ColorFormatCounts; ++active_attachments)
    {
        attachments[active_attachments] = color_attachments[active_attachments];
        ++active_attachments;
    }

    subpass.colorAttachmentCount = active_attachments ? active_attachments - 1 : 0;
    subpass.pColorAttachments    = color_attachment_refs;

    subpass.pDepthStencilAttachment = nullptr;
    u32 depth_stencil_count         = 0;
    if (InOutput.DepthStencilFormat != Format::UNDEFINED)
    {
        attachments[subpass.colorAttachmentCount] = depth_attachment;
        subpass.pDepthStencilAttachment           = &depth_attachment_reference;
        depth_stencil_count                       = 1;
    }

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount        = (active_attachments ? active_attachments - 1 : 0) + depth_stencil_count;
    render_pass_info.pAttachments           = attachments;
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass;

    VkRenderPass render_pass;
    vkcheck(vkCreateRenderPass(InDevice->Handle, &render_pass_info, nullptr, &render_pass));

    return render_pass;
}

void VulkanRenderPass::Destroy()
{
    SASSERT(device != nullptr);
    vkDestroyFramebuffer(device->Handle, framebuffer, nullptr);
    vkDestroyRenderPass(device->Handle, renderpass, nullptr);

    device      = nullptr;
    renderpass  = VK_NULL_HANDLE;
    framebuffer = VK_NULL_HANDLE;
    beginInfo   = {};
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
