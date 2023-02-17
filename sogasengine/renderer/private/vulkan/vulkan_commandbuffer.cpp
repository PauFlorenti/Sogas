#include "internal/vulkan/vulkan_commandbuffer.h"
#include "internal/vulkan/vulkan_device.h"

namespace Sogas
{
namespace Vk
{
VulkanCommandBuffer::VulkanCommandBuffer(const VulkanDevice* device) : device(device)
{
    SASSERT(device != nullptr);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        commandBuffers[i]  = VK_NULL_HANDLE;
        commandPools[i]    = VK_NULL_HANDLE;
        descriptorPools[i] = VK_NULL_HANDLE;
    }
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Destroy();
};

void VulkanCommandBuffer::Destroy()
{
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyCommandPool(device->Handle, commandPools[i], nullptr);
        vkDestroyDescriptorPool(device->Handle, descriptorPools[i], nullptr);
    }

    commandsToWait.clear();

    vkDestroySemaphore(device->Handle, semaphore, nullptr);

    activeRenderPass = nullptr;
    swapchain.reset();
    descriptorSetBound = nullptr;
}
} // namespace Vk
} // namespace Sogas
