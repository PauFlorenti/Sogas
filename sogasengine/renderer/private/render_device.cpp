#include "render_device.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
    std::shared_ptr<GPU_device> createVulkanDevice(void* device, std::vector<const char*> glfwExtensions)
    {
        return std::make_shared<Vk::VulkanDevice>(GraphicsAPI::Vulkan, device, glfwExtensions);
    }

    std::shared_ptr<GPU_device> createOpenGLDevice(void* /*device*/)
    {
        return nullptr;
    }

    std::shared_ptr<GPU_device> GPU_device::create(GraphicsAPI api, void* /*device*/, std::vector<const char*> extensions)
    {
        switch (api)
        {
        case GraphicsAPI::Vulkan:
        {
            std::shared_ptr<GPU_device> device(createVulkanDevice(nullptr, extensions));
            return device;
            break;
        }
        case GraphicsAPI::OpenGL:
            return createOpenGLDevice(nullptr);
            break;
        default:
            SFATAL("No valid api provided.");
            return nullptr;
            break;
        }
    }     
} // namespace Sogas
