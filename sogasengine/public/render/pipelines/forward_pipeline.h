#pragma once

#include "renderer/public/attachment.h"
#include "renderer/public/render_types.h"

namespace Sogas
{
struct Swapchain;
namespace Renderer
{
class GPU_device;
class Swapchain;

class ForwardPipeline
{
  public:
    explicit ForwardPipeline(std::shared_ptr<GPU_device> InRenderer = nullptr);
    ~ForwardPipeline(){};

    void update_constants();
    void render();
    void destroy();

  private:
    PipelineHandle pipeline;

    std::shared_ptr<GPU_device> renderer;
    std::shared_ptr<Texture>    colorBuffer;

    const u32 nLights = 10;
};
} // namespace Renderer
} // namespace Sogas