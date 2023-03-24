#pragma once

namespace Sogas
{
namespace Renderer
{
struct RenderPassHandle;
struct PipelineHandle;

class CommandBuffer
{
  public:
    virtual void init(u32 buffer_size, u32 submit_size, bool baked) = 0;
    virtual void finish()                                           = 0;

    // interface

    virtual void bind_pass(RenderPassHandle handle)   = 0;
    virtual void bind_pipeline(PipelineHandle handle) = 0;

    virtual void set_viewport() = 0;
    virtual void set_scissors() = 0;

    virtual void draw(u32 first_vertex, u32 vertex_count, u32 first_instance, u32 instance_count) = 0;

    virtual void reset() = 0;
};
} // namespace Renderer
} // namespace Sogas