
#include "render/module_render.h"

#include "render/vulkan/render.h"

namespace Sogas
{
    bool CRenderModule::Start () 
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        Renderer = std::make_unique<Sogas::Vk::CRender>();
        Renderer->Init();

        return true;
    }

    void CRenderModule::Stop() 
    {
        Renderer->Shutdown();
    }

    void CRenderModule::Update(f32 dt) 
    {
        Renderer->Update(dt);
    }

    void CRenderModule::Render() 
    {

    }

    void CRenderModule::RenderInMenu() 
    {

    }

    void CRenderModule::DoFrame()
    {
        Renderer->DrawFrame();
    }

    bool CRenderModule::CreateMesh(CMesh* mesh, std::vector<Vertex> vertices, PrimitiveTopology topology)
    {
        return Renderer->CreateMesh(mesh, vertices, topology);
    }

    void CRenderModule::Bind(const u32 renderId, PrimitiveTopology topology)
    {
        Renderer->Bind(renderId, topology);
    }

    void CRenderModule::Draw(const u32 vertexCount, const u32 vertexOffset)
    {
        Renderer->Draw(vertexCount, vertexOffset);
    }

    void CRenderModule::DrawIndexed(const u32 indexCount, const u32 indexOffset)
    {
        Renderer->DrawIndexed(indexCount, indexOffset);
    }

} // Sogas
