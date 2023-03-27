#include "vulkan/vulkan_swapchain.h"
#include "renderpass.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_texture.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

VulkanSwapchain::VulkanSwapchain(VulkanDevice* InDevice)
{
    device  = InDevice;
    texture = new Texture({});
}

VulkanSwapchain::~VulkanSwapchain()
{
    Destroy();
}

static void SetSurfaceFormat(std::shared_ptr<VulkanSwapchain> swapchain, VkPhysicalDevice physical_device, const std::vector<VkFormat> desired_surface_formats, VkColorSpaceKHR desired_color_space)
{
    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, swapchain->surface, &formatCount, nullptr);
    SASSERT_MSG(formatCount > 0, "\tNo formats available.");
    std::vector<VkSurfaceFormatKHR> supported_formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, swapchain->surface, &formatCount, supported_formats.data());

    bool found = false;
    for (const auto& format : supported_formats)
    {
        for (const auto& desired_format : desired_surface_formats)
        {
            if (format.format == desired_format && format.colorSpace == desired_color_space)
            {
                swapchain->surfaceFormat = format;
                swapchain->output.AddColor(ConvertFormat(format.format));
                found = true;
                break;
            }
        }

        if (found)
        {
            break;
        }
    }

    if (!found)
    {
        swapchain->surfaceFormat = supported_formats.at(0);
        swapchain->output.AddColor(ConvertFormat(supported_formats.at(0).format));
    }
}

static void SetPresentMode(std::shared_ptr<VulkanSwapchain> InSwapchain, VkPhysicalDevice InPhysicalDevice, VkPresentModeKHR InDesiredPresentMode)
{
    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, InSwapchain->surface, &presentModeCount, nullptr);
    SASSERT_MSG(presentModeCount > 0, "\tNo present modes available.");
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, InSwapchain->surface, &presentModeCount, presentModes.data());

    InSwapchain->presentMode = InDesiredPresentMode;
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            InSwapchain->presentMode = presentMode;
            break;
        }
    }
}

void VulkanSwapchain::CreateRenderPass(VulkanRenderPass* render_pass)
{
    // Create default render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = surfaceFormat.format;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment            = 0;
    colorAttachmentReference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    VulkanTexture*          depth_texture   = device->GetTextureResource(device->depth_texture);
    depthAttachment.format                  = depth_texture->descriptor.format;
    depthAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment            = 1;
    depthAttachmentReference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcAccessMask       = 0;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount        = 2;
    renderPassInfo.pAttachments           = attachments;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;

    vkcheck(vkCreateRenderPass(device->Handle, &renderPassInfo, nullptr, &render_pass->renderpass));

    // TODO Set render pass name

    VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferInfo.renderPass              = render_pass->renderpass;
    framebufferInfo.attachmentCount         = 2;
    framebufferInfo.width                   = width;
    framebufferInfo.height                  = height;
    framebufferInfo.layers                  = 1;

    // Create render targets
    u32 swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device->Handle, swapchain, &swapchainImageCount, nullptr);
    images.resize(swapchainImageCount);
    framebuffers.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device->Handle,
                            swapchain,
                            &swapchainImageCount,
                            images.data());

    VkImageView framebuffer_attachments[2];
    framebuffer_attachments[1] = depth_texture->image_view;

    for (size_t i = 0; i < swapchainImageCount; ++i)
    {
        framebuffer_attachments[0]   = imageViews.at(i);
        framebufferInfo.pAttachments = framebuffer_attachments;

        vkCreateFramebuffer(device->Handle, &framebufferInfo, nullptr, &framebuffers.at(i));
        // TODO set framebuffer name.
    }

    render_pass->width = width;
    render_pass->heigh = height;

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VulkanCommandBuffer* cmd = static_cast<VulkanCommandBuffer*>(device->GetInstantCommandBuffer());
    vkBeginCommandBuffer(cmd->command_buffer, &beginInfo);

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    for (auto& image : images)
    {
        TransitionImageLayout(device, cmd->command_buffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false);
    }
    TransitionImageLayout(device, cmd->command_buffer, depth_texture->texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, true);

    vkEndCommandBuffer(cmd->command_buffer);

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &cmd->command_buffer;

    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->GetGraphicsQueue());
}

bool VulkanSwapchain::Create(VulkanDevice* device, std::shared_ptr<VulkanSwapchain> swapchain)
{
    const auto& gpu = device->GetGPU();
    STRACE("\tCreating vulkan swapchain ...");

    std::vector<VkFormat> desired_surface_formats = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8_UNORM};
    const VkColorSpaceKHR desired_color_space     = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    swapchain->output.Reset();

    SetSurfaceFormat(swapchain, gpu, desired_surface_formats, desired_color_space);

    SetPresentMode(swapchain, gpu, VK_PRESENT_MODE_FIFO_KHR);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, swapchain->surface, &surfaceCapabilities);

    VkExtent2D extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width  = std::clamp(extent.width,
                                   surfaceCapabilities.minImageExtent.width,
                                   surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height,
                                   surfaceCapabilities.minImageExtent.height,
                                   surfaceCapabilities.maxImageExtent.height);
    }

    swapchain->width  = static_cast<u16>(extent.width);
    swapchain->height = static_cast<u16>(extent.height);

    u32 imageCount = surfaceCapabilities.minImageCount + 1;
    if (imageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface                  = swapchain->surface;
    swapchainCreateInfo.imageFormat              = swapchain->surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace          = swapchain->surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent              = extent;
    swapchainCreateInfo.imageArrayLayers         = 1;
    swapchainCreateInfo.minImageCount            = imageCount;
    swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform             = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode              = swapchain->presentMode;
    swapchainCreateInfo.clipped                  = VK_TRUE;
    swapchainCreateInfo.oldSwapchain             = swapchain->swapchain;
    // TODO should chose sharing mode depending on queueFamilyIndices ... probably passing VulkanDevice* into the
    // function.
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkcheck(vkCreateSwapchainKHR(device->Handle, &swapchainCreateInfo, nullptr, &swapchain->swapchain));

    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
    {
        STRACE("Destroying old swapchain.");
        vkDestroySwapchainKHR(device->Handle, swapchainCreateInfo.oldSwapchain, nullptr);
    }

    u32 image_count = 0;
    vkGetSwapchainImagesKHR(device->Handle, swapchain->swapchain, &image_count, nullptr);
    swapchain->imageViews.resize(image_count);
    swapchain->images.resize(image_count);
    vkGetSwapchainImagesKHR(device->Handle, swapchain->swapchain, &image_count, swapchain->images.data());

    for (u32 i = 0; i < image_count; ++i)
    {
        VkImageViewCreateInfo image_view_info       = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        image_view_info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format                      = swapchain->surfaceFormat.format;
        image_view_info.image                       = swapchain->images.at(i);
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.layerCount = 1;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.components.r                = VK_COMPONENT_SWIZZLE_R;
        image_view_info.components.g                = VK_COMPONENT_SWIZZLE_G;
        image_view_info.components.b                = VK_COMPONENT_SWIZZLE_B;
        image_view_info.components.a                = VK_COMPONENT_SWIZZLE_A;

        vkcheck(vkCreateImageView(device->Handle, &image_view_info, nullptr, &swapchain->imageViews.at(i)));
    }

    VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    if (vkCreateSemaphore(device->Handle, &semaphoreInfo, nullptr, &swapchain->presentCompleteSemaphore) !=
        VK_SUCCESS)
    {
        SERROR("Failed to create swapchain start semaphore!");
        return false;
    }

    if (vkCreateSemaphore(device->Handle, &semaphoreInfo, nullptr, &swapchain->renderCompleteSemaphore) !=
        VK_SUCCESS)
    {
        SERROR("Failed to create swapchain end semaphore!");
        return false;
    }

    STRACE("\tSwapchain image views created.");
    return true;
}

void VulkanSwapchain::Destroy()
{
    vkDeviceWaitIdle(device->Handle);

    vkDestroySemaphore(device->Handle, renderCompleteSemaphore, nullptr);
    vkDestroySemaphore(device->Handle, presentCompleteSemaphore, nullptr);

    for (auto& framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device->Handle, framebuffer, nullptr);
    }

    for (auto& imageView : imageViews)
    {
        vkDestroyImageView(device->Handle, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device->Handle, swapchain, nullptr);
    vkDestroySurfaceKHR(device->Instance, surface, nullptr);
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
