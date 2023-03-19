#include "render_device.h"
#include "public/sgs_memory.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{

DeviceDescriptor& DeviceDescriptor::SetWindow(void* InWindow, u16 InWidth, u16 InHeight)
{
    window = InWindow;
    width  = InWidth;
    height = InHeight;
    return *this;
}

DeviceDescriptor& DeviceDescriptor::SetAllocator(Memory::Allocator* InAllocator)
{
    allocator = InAllocator;
    return *this;
}

std::shared_ptr<GPU_device>
createVulkanDevice(std::vector<const char*> glfwExtensions)
{
    return std::make_shared<Vk::VulkanDevice>(GraphicsAPI::Vulkan, glfwExtensions);
}

std::shared_ptr<GPU_device> createOpenGLDevice(void* /*device*/)
{
    return nullptr;
}

std::shared_ptr<GPU_device> GPU_device::create(GraphicsAPI api, std::vector<const char*> extensions)
{
    switch (api)
    {
        case GraphicsAPI::Vulkan:
        {
            std::shared_ptr<GPU_device> device(createVulkanDevice(extensions));
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
} // namespace Renderer
} // namespace Sogas
