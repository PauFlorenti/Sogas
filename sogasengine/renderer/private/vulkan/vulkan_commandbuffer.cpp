
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_commandbuffer.h"
#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_swapchain.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

void VulkanCommandBufferResources::init(VulkanDevice* device)
{
    this->device = device;

    for (u32 i = 0; i < MAX_POOLS; ++i)
    {
        VkCommandPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_info.queueFamilyIndex        = device->GetFamilyQueueIndex();
        pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vkcheck(vkCreateCommandPool(device->Handle, &pool_info, nullptr, &command_pools[i]));
    }

    for (u32 i = 0; i < MAX_BUFFERS; ++i)
    {
        const u32 pool_index = pool_from_index(i);

        VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocate_info.commandPool                 = command_pools[pool_index];
        allocate_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount          = 1;

        vkcheck(vkAllocateCommandBuffers(device->Handle, &allocate_info, &command_buffers[i].command_buffer));

        command_buffers[i].device = device;
        command_buffers[i].handle = i;
        command_buffers[i].reset();
    }
}

void VulkanCommandBufferResources::shutdown()
{
    for (u32 i = 0; i < MAX_POOLS; ++i)
    {
        vkDestroyCommandPool(device->Handle, command_pools[i], nullptr);
    }
}

void VulkanCommandBufferResources::reset_pools(u32 frame_index)
{
    for (u32 i = 0; i < MAX_THREADS; ++i)
    {
        vkResetCommandPool(device->Handle, command_pools[frame_index * MAX_THREADS + i], 0);
    }
}

VulkanCommandBuffer* VulkanCommandBufferResources::get_command_buffer(u32 frame, bool begin)
{
    VulkanCommandBuffer* command_buffer = &command_buffers[frame * BUFFER_PER_POOL];

    if (begin)
    {
        command_buffer->reset();

        VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer->command_buffer, &begin_info);
    }

    return command_buffer;
}

VulkanCommandBuffer* VulkanCommandBufferResources::get_instant_command_buffer(u32 frame)
{
    return &command_buffers[frame * BUFFER_PER_POOL + 1];
}

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

    if (current_renderpass && (current_renderpass->type != RenderPassType::COMPUTE) && (current_renderpass != renderpass))
    {
        vkCmdEndRenderPass(command_buffer);
    }

    if (renderpass != current_renderpass && (renderpass->type != RenderPassType::COMPUTE))
    {
        auto swapchain = device->swapchain;

        VkRenderPassBeginInfo renderpass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderpass_begin_info.framebuffer           = renderpass->type == RenderPassType::SWAPCHAIN ? swapchain->framebuffers.at(device->GetFrameIndex()) : renderpass->framebuffer;
        renderpass_begin_info.renderPass            = renderpass->renderpass;

        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = {swapchain->width, swapchain->height};

        renderpass_begin_info.clearValueCount = 2;
        renderpass_begin_info.pClearValues    = clears;

        vkCmdBeginRenderPass(command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    current_renderpass = renderpass;
}

void VulkanCommandBuffer::bind_pipeline(PipelineHandle handle)
{
    VulkanPipeline* pipeline = device->GetPipelineResource(handle);
    vkCmdBindPipeline(command_buffer, pipeline->bind_point, pipeline->pipeline);

    current_pipeline = pipeline;
}

void VulkanCommandBuffer::bind_descriptor_set(DescriptorSetHandle handle, u32* offsets, u32 offsets_count)
{
    auto descriptor_set = device->GetDescriptorSetResource(handle);

    auto descriptor_set_layout = descriptor_set->layout;

    vkCmdBindDescriptorSets(command_buffer, current_pipeline->bind_point, current_pipeline->pipelineLayout, 0, 1, &descriptor_set->descriptorSet, 0, nullptr);
}

void VulkanCommandBuffer::bind_vertex_buffer(BufferHandle handle, u32 binding, u32 offset)
{
    auto buffer = device->GetBufferResource(handle);

    VkDeviceSize offsets = {offset};

    vkCmdBindVertexBuffers(command_buffer, binding, 1, &buffer->buffer, &offsets);
}

void VulkanCommandBuffer::bind_index_buffer(BufferHandle handle, u32 offset)
{
    auto buffer = device->GetBufferResource(handle);

    VkDeviceSize offsets = {offset};

    vkCmdBindIndexBuffer(command_buffer, buffer->buffer, offsets, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::set_viewport()
{
    VkViewport viewport;
    viewport.x        = 0;
    viewport.y        = 0;
    viewport.width    = device->swapchain->width;
    viewport.height   = device->swapchain->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::set_scissors()
{
    VkRect2D scissors;
    scissors.offset = {0, 0};
    scissors.extent = {device->swapchain->width, device->swapchain->height};
    vkCmdSetScissor(command_buffer, 0, 1, &scissors);
}

void VulkanCommandBuffer::draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count)
{
    vkCmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void VulkanCommandBuffer::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
{
    vkCmdDrawIndexed(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
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
