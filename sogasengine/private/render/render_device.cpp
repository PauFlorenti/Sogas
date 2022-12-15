#include "render_device.h"

#include "vulkan/vulkan_device.h"

namespace Sogas
{

    std::shared_ptr<GPU_device> createVulkanDevice(void* device)
    {
        return std::make_shared<Vk::VulkanDevice>(GraphicsAPI::Vulkan, device);
    }

    std::shared_ptr<GPU_device> createOpenGLDevice(void* /*device*/)
    {
        return nullptr; // std::make_shared();
        //return std::make_shared<OpenGLDevice>(device);
    }

    std::shared_ptr<GPU_device> GPU_device::create(GraphicsAPI api, void* device)
    {
        switch (api)
        {
        case GraphicsAPI::Vulkan:
            return createVulkanDevice(device);
            break;
        case GraphicsAPI::OpenGL:
            return createOpenGLDevice(device);
            break;
        default:
            SFATAL("No valid api provided.");
            return nullptr;
            break;
        }
    }     
} // Sogas
