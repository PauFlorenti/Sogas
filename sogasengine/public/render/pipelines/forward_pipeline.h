#pragma once

#include "renderer/public/attachment.h"
#include "renderer/public/render_types.h"

namespace Sogas
{
struct Swapchain;

namespace Renderer
{
class GPU_device;
}

class ForwardPipeline
{
  public:
    explicit ForwardPipeline(std::shared_ptr<Renderer::GPU_device> InRenderer = nullptr);
    ~ForwardPipeline(){};

    void update_constants();
    void render();
    void destroy();

  private:
    std::shared_ptr<Renderer::GPU_device> renderer;

    Renderer::BufferHandle              camera_buffer;
    Renderer::BufferHandle              mesh_buffer;
    Renderer::BufferHandle              light_buffer;
    Renderer::PipelineHandle            pipeline;
    Renderer::DescriptorSetHandle       descriptorSet;
    Renderer::DescriptorSetLayoutHandle descriptorLayout;

    const u32 nLights = 10;
};
} // namespace Sogas