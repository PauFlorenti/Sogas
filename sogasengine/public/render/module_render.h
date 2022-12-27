#pragma once

#include "modules/module.h"
#include "render/vulkan/vulkan_device.h"

namespace Sogas
{
    class CRenderModule : public IModule
    {
    public:
        CRenderModule(const std::string name) : IModule(name){}

        bool Start() override;
        void Stop() override;
        void Update(f32 /*dt*/) override;
        void Render() override;
        void RenderInMenu() override;

        std::shared_ptr<GPU_device> GetGraphicsDevice() const { return renderer; }

        void DoFrame();

    private:
        std::shared_ptr<GPU_device> renderer;
        std::shared_ptr<Swapchain> swapchain;
    };
} // Sogas
