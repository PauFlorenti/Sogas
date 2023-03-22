#pragma once

#include "renderer/public/attachment.h"
#include "renderer/public/render_types.h"

namespace Sogas
{
struct Swapchain;
namespace Renderer
{
class GPU_device;
class Buffer;
class Swapchain;
class RenderPass;
class Texture;

class ForwardPipeline
{
  public:
    explicit ForwardPipeline(std::shared_ptr<GPU_device> InRenderer = nullptr);
    ~ForwardPipeline(){};

    void update_constants();
    void render(std::shared_ptr<Swapchain> swapchain);
    void destroy();

  private:
    std::shared_ptr<GPU_device> renderer;
    Pipeline                    pipeline;
    Renderer::RenderPass*       forwardRenderPass;
    Shader                      forwardShaders[2]; // 0 vs, 1 ps
    Pipeline                    presentPipeline;
    Shader                      presentShaders[2];

    AttachmentFramebuffer colorAttachment;
    AttachmentFramebuffer depthAttachment;

    std::shared_ptr<Texture> colorBuffer;

    const u32 nLights = 10;
};
} // namespace Renderer
} // namespace Sogas