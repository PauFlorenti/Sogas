#pragma once

#include "commandbuffer.h"
#include "vulkan_types.h"
namespace Sogas
{
namespace Renderer
{
namespace Vk
{

class VulkanDevice;
class VulkanRenderPass;
class VulkanPipeline;

class VulkanCommandBuffer : public CommandBuffer
{
    void init(u32 buffer_size, u32 submit_size, bool baked) override;
    void finish() override;

    // interface

    void bind_pass(RenderPassHandle handle) override;
    void bind_pipeline(PipelineHandle handle) override;

    void draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count) override;

    void reset() override;

    VulkanDevice* device = nullptr;

    VkCommandBuffer command_buffer;

    VulkanRenderPass* current_renderpass = nullptr;
    VulkanPipeline*   current_pipeline   = nullptr;
    VkClearValue      clears[2];
    bool              is_recording;

    u32 handle;

    u32            current_command;
    ResourceHandle resource_handle;
    u32            buffer_size = 0;
    bool           baked       = false;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
