
#include "render/module_render.h"
#include "render/render_device.h"
#include "render/render_manager.h"

// ! TEMP
#include "components/camera_component.h"

namespace Sogas
{
    bool CRenderModule::Start () 
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        Renderer = GPU_device::create(GraphicsAPI::Vulkan, nullptr);
        Renderer->Init();

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
            CEntity* camera_entity = getEntityByName("camera");
            SASSERT(camera_entity);
            const TCompCamera* camera = camera_entity->Get<TCompCamera>();
            
            Renderer->activateCamera(camera);

            // Render all solid objects
            RenderManager.RenderAll(CHandle(), DrawChannel::SOLID);
            Renderer->endFrame();
        }
    }

    void CRenderModule::ActivateObject(const glm::mat4& model, const glm::vec4& color)
    {
        Renderer->activateObject(model, color);
    }

} // Sogas
