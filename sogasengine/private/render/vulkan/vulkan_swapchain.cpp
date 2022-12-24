#include "render/vulkan/vulkan_swapchain.h"

namespace Sogas
{
namespace Vk
{
    VulkanSwapchain::~VulkanSwapchain()
    {
    }

    bool VulkanSwapchain::Create(
        const VkDevice& device,
        const VkPhysicalDevice& gpu,
        VulkanSwapchain* internalState
    )
    {
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

        internalState->surfaceFormat.format = Vk::ConvertFormat(internalState->descriptor.format);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, internalState->surface, &formatCount, formats.data());

        {
            internalState->surfaceFormat = formats.at(0);
            for (const auto &format : formats)
            {
                if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == internalState->surfaceFormat.format)
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
            for (const auto &presentMode : presentModes)
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
            internalState->extent.width  = internalState->descriptor.width;
            internalState->extent.height = internalState->descriptor.height;
            internalState->extent.width  = std::clamp(internalState->extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            internalState->extent.height = std::clamp(internalState->extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }

        u32 imageCount = surfaceCapabilities.minImageCount + 1;
        if (imageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
            imageCount = surfaceCapabilities.maxImageCount;

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swapchainCreateInfo.surface = internalState->surface;
        swapchainCreateInfo.imageFormat = internalState->surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = internalState->surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = internalState->extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = internalState->presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = internalState->swapchain;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        /*
        u32 queueFamilyIndices[] = {GraphicsFamily, PresentFamily};
        if (GraphicsFamily != PresentFamily)
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        */

        if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &internalState->swapchain) != VK_SUCCESS)
        {
            SFATAL("\tFailed to create swapchain!");
            return false;
        }

        if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
        {
            STRACE("Destroying old swapchain.");
            vkDestroySwapchainKHR(device, swapchainCreateInfo.oldSwapchain, nullptr);
        }

        u32 swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(device, internalState->swapchain, &swapchainImageCount, nullptr);
        internalState->images.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(device, internalState->swapchain, &swapchainImageCount, internalState->images.data());

        STRACE("\tCreating swapchain image views.");
        internalState->imageViews.resize(swapchainImageCount);
        for (u32 i = 0; i < swapchainImageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            imageViewCreateInfo.image = internalState->images.at(i);
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = internalState->surfaceFormat.format;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;

            if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &internalState->imageViews.at(i)) != VK_SUCCESS)
            {
                std::cout << "\tFailed to create image view!\n";
                return false;
            }
        }

        STRACE("\tSwapchain image views created.");
        return true;
    }
} // Vk
} // Sogas
