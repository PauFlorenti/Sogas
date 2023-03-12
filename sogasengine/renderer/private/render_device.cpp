#include "render_device.h"
#include "public/sgs_memory.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
std::shared_ptr<GPU_device>
createVulkanDevice(void* device, std::vector<const char*> glfwExtensions, Memory::Allocator* InAllocator)
{
    return std::make_shared<Vk::VulkanDevice>(GraphicsAPI::Vulkan, device, glfwExtensions, InAllocator);
}

std::shared_ptr<GPU_device> createOpenGLDevice(void* /*device*/)
{
    return nullptr;
}

std::shared_ptr<GPU_device> GPU_device::create(GraphicsAPI api, void* /*device*/, std::vector<const char*> extensions, Memory::Allocator* InAllocator)
{
    switch (api)
    {
    case GraphicsAPI::Vulkan:
    {
        std::shared_ptr<GPU_device> device(createVulkanDevice(nullptr, extensions, InAllocator));
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
