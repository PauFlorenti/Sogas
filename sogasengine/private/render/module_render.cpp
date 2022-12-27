
#include "render/module_render.h"
#include "render/render_device.h"
#include "render/render_manager.h"

// ! TEMP
#include "application.h"
#include "components/camera_component.h"
#include "resources/mesh.h"

namespace Sogas
{
    Pipeline pipeline;
    Shader shaders[2];

    CMesh square;


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

        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
        };

        square.Create(std::move(vertices), std::vector<u32>(), PrimitiveTopology::TRIANGLELIST);

        // Init global buffers
        renderer->CreateShader(ShaderStage::VERTEX, "test.vert.spv", &shaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "test.frag.spv", &shaders[1]);

        PushConstantDescriptor modelPushConstant;
        modelPushConstant.offset = 0;
        modelPushConstant.size   = sizeof(glm::mat4);
        modelPushConstant.stage  = ShaderStage::VERTEX;

        PipelineDescriptor psoDesc;
        psoDesc.vs = &shaders[0];
        psoDesc.ps = &shaders[1];
        psoDesc.vertexDeclaration = "PosColor";
        psoDesc.pushConstantDesc.push_back(modelPushConstant);

        renderer->CreatePipeline(&psoDesc, &pipeline, &swapchain->renderpass);

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
        CommandBuffer cmd = renderer->BeginCommandBuffer();
        renderer->BindPipeline(&pipeline, cmd);
        renderer->BeginRenderPass(swapchain.get(), cmd);

        renderer->BindVertexBuffer(&square.vertexBuffer, cmd);
        renderer->ActivateObject(glm::translate(glm::mat4(1), glm::vec3(1, 0, 0)), glm::vec4(1.0f), cmd);
        renderer->Draw(square.vertexCount, 0, cmd);

        renderer->EndRenderPass(cmd);
        renderer->SubmitCommandBuffers();
    }
} // Sogas
