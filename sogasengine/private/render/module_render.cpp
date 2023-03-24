
#include "render/module_render.h"
#include "buffer.h"
#include "render/pipelines/forward_pipeline.h"
#include "render/render_manager.h"

// ! TEMP
#include "application.h"

namespace Sogas
{

std::shared_ptr<Renderer::ForwardPipeline> forwardPipeline;

bool CRenderModule::Start()
{
    // Start ImGui

    allocator.init(4 * 1024 * 1024);

    // Start selected renderer. Vulkan only at the moment and by default.
    u32                      extensionsCount = 0;
    const char**             extensions      = glfwGetRequiredInstanceExtensions(&extensionsCount);
    std::vector<const char*> extensions_vector(extensions, extensions + extensionsCount);

    renderer = Renderer::GPU_device::create(Renderer::GraphicsAPI::Vulkan, extensions_vector);

    i32 width, height;
    CApplication::Get()->GetWindowSize(&width, &height);
    Renderer::DeviceDescriptor dc;
    dc.SetWindow(CApplication::Get()->GetWindow(), static_cast<u16>(width), static_cast<u16>(height)).SetAllocator(&allocator);
    renderer->Init(dc);

    forwardPipeline = std::make_shared<Renderer::ForwardPipeline>(renderer);

    return true;
}

void CRenderModule::Stop()
{
    //forwardPipeline->destroy();
    renderer->shutdown();
}

void CRenderModule::Update(f32 /*dt*/)
{
    // if (swapchain->resized)
    // {
    //     i32 width, height;
    //     glfwGetWindowSize(CApplication::Get()->GetWindow(), &width, &height);
    //     renderer->SetWindowSize(swapchain, width, height);
    // }
}

void CRenderModule::Render() {}

void CRenderModule::RenderInMenu() {}

void CRenderModule::DoFrame()
{
    forwardPipeline->render();
}
} // namespace Sogas
