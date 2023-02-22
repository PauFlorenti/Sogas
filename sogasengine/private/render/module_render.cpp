
#include "render/module_render.h"
#include "buffer.h"
#include "render/pipelines/forward_pipeline.h"
#include "render/render_manager.h"
#include "swapchain.h"

// ! TEMP
#include "application.h"

namespace Sogas
{

std::shared_ptr<Renderer::ForwardPipeline> forwardPipeline;

bool CRenderModule::Start()
{
    // Start ImGui

    // Start selected renderer. Vulkan only at the moment and by default.
    u32                      extensionsCount = 0;
    const char**             extensions      = glfwGetRequiredInstanceExtensions(&extensionsCount);
    std::vector<const char*> extensions_vector(extensions, extensions + extensionsCount);
    renderer = Renderer::GPU_device::create(Renderer::GraphicsAPI::Vulkan, nullptr, extensions_vector);
    renderer->Init();

    i32 width, height;
    CApplication::Get()->GetWindowSize(&width, &height);

    Renderer::SwapchainDescriptor swapchain_descriptor;
    swapchain_descriptor.format = Renderer::Format::R32G32B32A32_SFLOAT;
    swapchain_descriptor.width  = width;
    swapchain_descriptor.height = height;
    swapchain                   = std::make_shared<Renderer::Swapchain>(std::move(swapchain_descriptor));
    renderer->CreateSwapchain(swapchain, CApplication::Get()->GetWindow());

    forwardPipeline = std::make_shared<Renderer::ForwardPipeline>(renderer, swapchain);

    return true;
}

void CRenderModule::Stop()
{
    forwardPipeline->destroy();
    swapchain->Destroy();
    renderer->shutdown();
}

void CRenderModule::Update(f32 /*dt*/)
{
    if (swapchain->resized)
    {
        i32 width, height;
        glfwGetWindowSize(CApplication::Get()->GetWindow(), &width, &height);
        renderer->SetWindowSize(swapchain, width, height);
    }
}

void CRenderModule::Render() {}

void CRenderModule::RenderInMenu() {}

void CRenderModule::DoFrame()
{
    forwardPipeline->render(swapchain);
}
} // namespace Sogas
