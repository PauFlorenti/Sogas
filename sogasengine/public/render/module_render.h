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

        void DoFrame();

        bool CreateMesh(CMesh* mesh, const std::vector<Vertex>& vertices, PrimitiveTopology topology);
        bool CreateMesh(CMesh* mesh, const std::vector<Vertex>& vertices, const std::vector<u32>& indices, PrimitiveTopology topology);
        void Bind(const CMesh* mesh);
        void Draw(const CMesh* mesh);
        //void Draw(const u32 vertexCount, const u32 vertexOffset);
        //void DrawIndexed(const u32 indexCount, const u32 indexOffset);
        void ActivateObject(const glm::mat4& model, const glm::vec4& color);

    private:
        std::shared_ptr<GPU_device> Renderer;
    };
} // Sogas
