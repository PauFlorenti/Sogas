#pragma once

#include "commandbuffer.h"
#include "device_resources.h"
#include <vulkan/vulkan.h>

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
  public:
    void init(u32 buffer_size, u32 submit_size, bool baked) override;
    void set_device(VulkanDevice* device)
    {
        device = device;
    }
    void finish() override;

    // interface

    void bind_pass(RenderPassHandle handle) override;
    void bind_pipeline(PipelineHandle handle) override;

    void set_viewport() override;
    void set_scissors() override;

    void bind_vertex_buffer(BufferHandle handle, u32 binding, u32 offset) override;
    void bind_index_buffer(BufferHandle handle, u32 offset) override;
    void bind_descriptor_set(DescriptorSetHandle handle, u32* offsets, u32 offsets_count) override;

    void draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count) override;
    void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) override;

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

class VulkanCommandBufferResources
{
  public:
    void init(VulkanDevice* device);
    void shutdown();

    void reset_pools(u32 frame_index);

    VulkanCommandBuffer* get_command_buffer(u32 frame, bool begin);
    VulkanCommandBuffer* get_instant_command_buffer(u32 frame);

    static u16 pool_from_index(u32 index)
    {
        return static_cast<u16>(index) / BUFFER_PER_POOL;
    }

    static const u16 MAX_THREADS     = 1;
    static const u16 MAX_POOLS       = MAX_SWAPCHAIN_IMAGES * MAX_THREADS;
    static const u16 BUFFER_PER_POOL = 4;
    static const u16 MAX_BUFFERS     = BUFFER_PER_POOL * MAX_POOLS;

    VulkanDevice*       device = nullptr;
    VkCommandPool       command_pools[MAX_POOLS];
    VulkanCommandBuffer command_buffers[MAX_BUFFERS];
    u8                  next_free_per_thread_frame[MAX_POOLS];
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
