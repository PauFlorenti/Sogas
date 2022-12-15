
#include "render/module_render.h"
#include "render/render_manager.h"
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
        if(Renderer->PrepareFrame())
        {
            // Activate main camera
            Renderer->ActivateCamera();

            // Render all solid objects
            RenderManager.RenderAll(CHandle(), DrawChannel::SOLID);
            //Renderer->DrawFrame();
            Renderer->EndFrame();
        }
    }

    bool CRenderModule::CreateMesh(
        CMesh* mesh, 
        const std::vector<Vertex>& vertices, 
        PrimitiveTopology topology)
    {
        return Renderer->CreateMesh(mesh, vertices, topology);
    }

    bool CRenderModule::CreateMesh(
        CMesh* mesh, 
        const std::vector<Vertex>& vertices, 
        const std::vector<u32>& indices, 
        PrimitiveTopology topology)
    {
        return Renderer->CreateMesh(mesh, vertices, indices, topology);
    }

    void CRenderModule::Bind(const u32 renderId, PrimitiveTopology topology, const bool indexed)
    {
        Renderer->Bind(renderId, topology, indexed);
    }

    void CRenderModule::Draw(const u32 vertexCount, const u32 vertexOffset)
    {
        Renderer->Draw(vertexCount, vertexOffset);
    }

    void CRenderModule::DrawIndexed(const u32 indexCount, const u32 indexOffset)
    {
        Renderer->DrawIndexed(indexCount, indexOffset);
    }

    void CRenderModule::ActivateObject(const glm::mat4& model, const glm::vec4& color)
    {
        Renderer->ActivateObject(model, color);
    }

} // Sogas
