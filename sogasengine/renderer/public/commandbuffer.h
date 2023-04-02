#pragma once

namespace Sogas
{
namespace Renderer
{
struct BufferHandle;
struct PipelineHandle;
struct RenderPassHandle;
struct DescriptorSetHandle;

class CommandBuffer
{
  public:
    virtual void init(u32 buffer_size, u32 submit_size, bool baked) = 0;
    virtual void finish()                                           = 0;

    // interface

    virtual void bind_pass(RenderPassHandle handle)   = 0;
    virtual void bind_pipeline(PipelineHandle handle) = 0;
    virtual void bind_descriptor_set(DescriptorSetHandle handle, u32* offsets, u32 offsets_count) = 0;

    virtual void bind_vertex_buffer(BufferHandle handle, u32 binding, u32 offset) = 0;
    virtual void bind_index_buffer(BufferHandle handle, u32 offset) = 0;

    virtual void set_viewport() = 0;
    virtual void set_scissors() = 0;

    virtual void draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count) = 0;
    virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) = 0;

    virtual void reset() = 0;
};
} // namespace Renderer
} // namespace Sogas