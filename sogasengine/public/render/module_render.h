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

        std::shared_ptr<GPU_device> GetGraphicsDevice() const { return Renderer; }

        void DoFrame();

        void ActivateObject(const glm::mat4& model, const glm::vec4& color);

    private:
        std::shared_ptr<GPU_device> Renderer;
    };
} // Sogas
