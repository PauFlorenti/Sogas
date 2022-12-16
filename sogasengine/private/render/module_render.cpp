
#include "render/module_render.h"
#include "render/render_device.h"
#include "render/render_manager.h"

namespace Sogas
{
    bool CRenderModule::Start () 
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        Renderer = GPU_device::create(GraphicsAPI::Vulkan, nullptr);
        Renderer->init();

        return true;
    }

    void CRenderModule::Stop() 
    {
        Renderer->shutdown();
    }

    void CRenderModule::Update(f32 /*dt*/) 
    {
    }

    void CRenderModule::Render() 
    {

    }

    void CRenderModule::RenderInMenu() 
    {

    }

    void CRenderModule::DoFrame()
    {
        if(Renderer->beginFrame())
        {
            // Activate main camera
            // ! TEMPORAL
            Renderer->activateCamera();

            // Render all solid objects
            RenderManager.RenderAll(CHandle(), DrawChannel::SOLID);
            //Renderer->DrawFrame();
            Renderer->endFrame();
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

    void CRenderModule::Bind(const CMesh* mesh)
    {
        Renderer->bind(mesh);
        //Renderer->Bind(renderId, topology, indexed);
    }

    void CRenderModule::Draw(const CMesh* mesh)
    {
        Renderer->draw(mesh);
    }

/*
    void CRenderModule::DrawIndexed(const u32 indexCount, const u32 indexOffset)
    {
        //Renderer->DrawIndexed(indexCount, indexOffset);
    }
*/

    void CRenderModule::ActivateObject(const glm::mat4& model, const glm::vec4& color)
    {
        Renderer->activateObject(model, color);
    }

} // Sogas
