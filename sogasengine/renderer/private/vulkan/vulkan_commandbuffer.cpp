#include "internal/vulkan/vulkan_commandbuffer.h"
#include "internal/vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
void VulkanCommandBuffer::init(u32 buffer_size, u32 submit_size, bool baked)
{
    buffer_size = buffer_size;
    baked       = baked;

    reset();
}

void VulkanCommandBuffer::finish()
{
    is_recording = false;
}

// interface

void VulkanCommandBuffer::bind_pass(RenderPassHandle handle)
{
    is_recording = true;

    VulkanRenderPass* renderpass = device->GetRenderPassResource(handle);
}

void VulkanCommandBuffer::bind_pipeline(PipelineHandle handle)
{
}

void VulkanCommandBuffer::draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count)
{
}

void VulkanCommandBuffer::reset()
{
    is_recording       = false;
    current_renderpass = nullptr;
    current_pipeline   = nullptr;
    current_command    = 0;
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
