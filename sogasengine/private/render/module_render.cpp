
#include "render/module_render.h"
#include "render/render_device.h"
#include "render/render_manager.h"

// ! TEMP
#include "application.h"
#include "components/camera_component.h"

namespace Sogas
{
    bool CRenderModule::Start () 
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        renderer = GPU_device::create(GraphicsAPI::Vulkan, nullptr);
        renderer->Init();

        i32 width, height;
        CApplication::Get()->GetWindowSize(&width, &height);

        swapchain = std::make_shared<Swapchain>();
        SwapchainDescriptor desc;
        desc.format = Format::R32G32B32A32_SFLOAT;
        desc.width = width;
        desc.height = height;
        renderer->CreateSwapchain(desc, swapchain.get());

        // Init global buffers

        return true;
    }

    void CRenderModule::Stop() 
    {
        renderer->shutdown();
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
        /*
        if(renderer->beginFrame())
        {
            // 

            // Activate main camera
            // ! TEMPORAL
            CEntity* camera_entity = getEntityByName("camera");
            SASSERT(camera_entity);
            const TCompCamera* camera = camera_entity->Get<TCompCamera>();
            
            renderer->activateCamera(camera);

            // Render all solid objects
            RenderManager.RenderAll(CHandle(), DrawChannel::SOLID);
            renderer->endFrame();
        }
        */
    }

    void CRenderModule::ActivateObject(const glm::mat4& model, const glm::vec4& color)
    {
        renderer->activateObject(model, color);
    }

} // Sogas
