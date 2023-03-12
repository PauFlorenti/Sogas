#pragma once

#include "modules/module.h"
#include "renderer/public/render_device.h"
#include "sgs_memory.h"

namespace Sogas
{
struct Swapchain;
class CRenderModule : public IModule
{
  public:
    CRenderModule(const std::string name)
        : IModule(name)
    {}

    bool Start() override;
    void Stop() override;
    void Update(f32 /*dt*/) override;
    void Render() override;
    void RenderInMenu() override;

    std::shared_ptr<Renderer::GPU_device> GetGraphicsDevice() const { return renderer; }

    void DoFrame();

  private:
    std::shared_ptr<Renderer::GPU_device> renderer;
    std::shared_ptr<Renderer::Swapchain>  swapchain;

    Memory::StackAllocator allocator;
};
} // namespace Sogas
