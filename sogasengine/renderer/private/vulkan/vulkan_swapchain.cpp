#include "vulkan/vulkan_swapchain.h"
#include "renderpass.h"
#include "swapchain.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_renderpass.h"

namespace Sogas
{
namespace Vk
{

VulkanSwapchain::VulkanSwapchain(const VulkanDevice* InDevice) : device(InDevice) {}

VulkanSwapchain::~VulkanSwapchain()
{
    Release();
}

bool VulkanSwapchain::Create(const VulkanDevice* device, std::shared_ptr<Renderer::Swapchain> swapchain)
{
    auto        internalState = VulkanSwapchain::ToInternal(swapchain);
    const auto& gpu           = device->GetGPU();
    STRACE("\tCreating vulkan swapchain ...");
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, internalState->surface, &surfaceCapabilities);

    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, internalState->surface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        SERROR("\tNo surface formats found");
        return false;
    }

    internalState->surfaceFormat.format = Vk::ConvertFormat(swapchain->GetDescriptor().format);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, internalState->surface, &formatCount, formats.data());

    {
        internalState->surfaceFormat = formats.at(0);
        for (const auto& format : formats)
        {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
                format.format == internalState->surfaceFormat.format)
            {
                internalState->surfaceFormat = format;
                break;
            }
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
            {
                internalState->surfaceFormat = format;
            }
        }
    }

    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, internalState->surface, &presentModeCount, nullptr);

    if (presentModeCount == 0)
    {
        SERROR("\tNo present modes available.");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, internalState->surface, &presentModeCount, presentModes.data());

    {
        internalState->presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                internalState->presentMode = presentMode;
                break;
            }
        }
    }

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        internalState->extent = surfaceCapabilities.currentExtent;
    }
    else
    {
        const auto& swapchain_descriptor = swapchain->GetDescriptor();
        internalState->extent.width      = swapchain_descriptor.width;
        internalState->extent.height     = swapchain_descriptor.height;
        internalState->extent.width      = std::clamp(internalState->extent.width,
                                                 surfaceCapabilities.minImageExtent.width,
                                                 surfaceCapabilities.maxImageExtent.width);
        internalState->extent.height     = std::clamp(internalState->extent.height,
                                                  surfaceCapabilities.minImageExtent.height,
                                                  surfaceCapabilities.maxImageExtent.height);
    }

    u32 imageCount = surfaceCapabilities.minImageCount + 1;
    if (imageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
        imageCount = surfaceCapabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface                  = internalState->surface;
    swapchainCreateInfo.imageFormat              = internalState->surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace          = internalState->surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent              = internalState->extent;
    swapchainCreateInfo.imageArrayLayers         = 1;
    swapchainCreateInfo.minImageCount            = imageCount;
    swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform             = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode              = internalState->presentMode;
    swapchainCreateInfo.clipped                  = VK_TRUE;
    swapchainCreateInfo.oldSwapchain             = internalState->swapchain;
    // TODO should chose sharing mode depending on queueFamilyIndices ... probably passing VulkanDevice* into the
    // function.
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateSwapchainKHR(device->Handle, &swapchainCreateInfo, nullptr, &internalState->swapchain) != VK_SUCCESS)
    {
        SFATAL("\tFailed to create swapchain!");
        return false;
    }

    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
    {
        STRACE("Destroying old swapchain.");
        vkDestroySwapchainKHR(device->Handle, swapchainCreateInfo.oldSwapchain, nullptr);
    }

    // Create default render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = internalState->surfaceFormat.format;
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

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentReference;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcAccessMask       = 0;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;

    swapchain->renderpass->internalState = new VulkanRenderPass(device);
    auto renderPassInternal              = VulkanRenderPass::ToInternal(swapchain->GetRenderpass());

    internalState->texture.descriptor.width  = internalState->extent.width;
    internalState->texture.descriptor.height = internalState->extent.height;
    internalState->texture.descriptor.depth  = 1;
    internalState->texture.descriptor.format = swapchain->GetDescriptor().format;

    swapchain->renderpass->AddAttachment(Attachment::RenderTarget(&internalState->texture));

    if (vkCreateRenderPass(device->Handle, &renderPassInfo, nullptr, &renderPassInternal->renderpass) != VK_SUCCESS)
    {
        SFATAL("Failed to create swapchain default renderpass.");
        return false;
    }

    // Create render targets
    u32 swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device->Handle, internalState->swapchain, &swapchainImageCount, nullptr);
    internalState->images.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device->Handle,
                            internalState->swapchain,
                            &swapchainImageCount,
                            internalState->images.data());

    STRACE("\tCreating swapchain image views.");
    internalState->imageViews.resize(swapchainImageCount);
    internalState->framebuffers.resize(swapchainImageCount);
    for (u32 i = 0; i < swapchainImageCount; i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCreateInfo.image                           = internalState->images.at(i);
        imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format                          = internalState->surfaceFormat.format;
        imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;

        if (vkCreateImageView(device->Handle, &imageViewCreateInfo, nullptr, &internalState->imageViews.at(i)) !=
            VK_SUCCESS)
        {
            std::cout << "\tFailed to create image view!\n";
            return false;
        }

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.width                   = internalState->extent.width;
        framebufferInfo.height                  = internalState->extent.height;
        framebufferInfo.renderPass              = renderPassInternal->renderpass;
        framebufferInfo.attachmentCount         = 1;
        framebufferInfo.pAttachments            = &internalState->imageViews[i];
        framebufferInfo.layers                  = 1;

        if (vkCreateFramebuffer(device->Handle, &framebufferInfo, nullptr, &internalState->framebuffers.at(i)) !=
            VK_SUCCESS)
        {
            SERROR("Failed to create framebuffer");
            return false;
        }
    }

    VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    if (vkCreateSemaphore(device->Handle, &semaphoreInfo, nullptr, &internalState->presentCompleteSemaphore) !=
        VK_SUCCESS)
    {
        SERROR("Failed to create swapchain start semaphore!");
        return false;
    }

    if (vkCreateSemaphore(device->Handle, &semaphoreInfo, nullptr, &internalState->renderCompleteSemaphore) !=
        VK_SUCCESS)
    {
        SERROR("Failed to create swapchain end semaphore!");
        return false;
    }

    STRACE("\tSwapchain image views created.");
    return true;
}

void VulkanSwapchain::Release()
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
} // namespace Sogas
