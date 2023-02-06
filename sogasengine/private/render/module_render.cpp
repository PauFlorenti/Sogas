
#include "buffer.h"
#include "render_device.h"
#include "render/module_render.h"
#include "render/render_manager.h"

// ! TEMP
#include "application.h"
#include "components/camera_component.h"
#include "components/light_point_component.h"
#include "resources/mesh.h"

namespace Sogas
{
    Pipeline forwardPipeline; // Forward pipeline
    Pipeline presentPipeline;
    RenderPass forwardRenderPass;
    Shader forwardShaders[2]; // 0 vs, 1 ps
    Shader presentShaders[2];
    std::shared_ptr<Renderer::Buffer> constantBuffer;
    std::shared_ptr<Renderer::Buffer> lightBuffer;
    std::shared_ptr<Renderer::Buffer> quadIdxBuffer;
    std::shared_ptr<Renderer::Buffer> quadBuffer;

    std::shared_ptr<Texture> colorBuffer;
    AttachmentFramebuffer colorAttachment;
    AttachmentFramebuffer depthAttachment;
    const u32 nLights = 10;

    std::vector<Vertex> quad = {
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    };

    std::vector<u32> quadIdx = {0, 1, 2, 2, 3, 0};

    struct ConstantsCamera
    {
        glm::mat4 camera_projection;
        glm::mat4 camera_view;
        glm::mat4 camera_view_projection;
        glm::mat4 camera_inverse_view_projection;

        // Camera forward
        // Camera z far
        // Camera position
        // Camera Right
        // Camera Up
    };

    struct Light
    {
        glm::vec3 color;
        f32 intensity;
        glm::vec3 position;
        f32 radius;
    };

    Light lights[nLights];

    struct ConstantsMesh
    {
        glm::mat4 model;
        glm::vec4 color;
    };

    bool CRenderModule::Start()
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        u32 extensionsCount = 0;
        const char **extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        std::vector<const char *> extensions_vector(extensions, extensions + extensionsCount);
        renderer = GPU_device::create(GraphicsAPI::Vulkan, nullptr, extensions_vector);
        renderer->Init();

        i32 width, height;
        CApplication::Get()->GetWindowSize(&width, &height);

        // TODO make swapchain unique ??
        swapchain = std::make_shared<Swapchain>();
        SwapchainDescriptor desc;
        desc.format = Format::R32G32B32A32_SFLOAT;
        desc.width = width;
        desc.height = height;
        renderer->CreateSwapchain(desc, swapchain, CApplication::Get()->GetWindow());

        // Create shaders
        renderer->CreateShader(ShaderStage::VERTEX, "forward.vert.spv", &forwardShaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "forward.frag.spv", &forwardShaders[1]);
        renderer->CreateShader(ShaderStage::VERTEX, "quad.vert.spv", &presentShaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "quad.frag.spv", &presentShaders[1]);

        colorAttachment.format = Format::R8G8B8A8_SRGB;
        colorAttachment.usage = BindPoint::RENDER_TARGET;
        renderer->CreateAttachment(&colorAttachment);

        depthAttachment.format = Format::D24_UNORM_S8_UINT;
        depthAttachment.usage = BindPoint::DEPTH_STENCIL;
        renderer->CreateAttachment(&depthAttachment);

        RenderPassDescriptor rpDesc;
        rpDesc.attachments.push_back(Attachment::RenderTarget(nullptr, &colorAttachment));
        rpDesc.attachments.push_back(Attachment::DepthStencil(nullptr, &depthAttachment));
        renderer->CreateRenderPass(&rpDesc, &forwardRenderPass);

        DepthStencilState depthState;
        depthState.compareOp = CompareOperations::LESS_OR_EQUAL;
        depthState.depthTestEnabled = true;
        depthState.writeDepthEnabled = true;

        PipelineDescriptor fwdDesc;
        fwdDesc.vs = &forwardShaders[0];
        fwdDesc.ps = &forwardShaders[1];
        fwdDesc.vertexDeclaration = "PosNormalUvColor";
        fwdDesc.depthStencilState = &depthState;
        renderer->CreatePipeline(&fwdDesc, &forwardPipeline, &forwardRenderPass);

        PipelineDescriptor psoDesc;
        psoDesc.vs = &presentShaders[0];
        psoDesc.ps = &presentShaders[1];
        psoDesc.vertexDeclaration = "PosNormalUvColor";
        renderer->CreatePipeline(&psoDesc, &presentPipeline, &swapchain->renderpass);

        PushConstantDescriptor modelPushConstant;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ConstantsMesh);
        modelPushConstant.stage = ShaderStage::VERTEX;

        Renderer::BufferDescriptor constantBufferDesc;
        constantBufferDesc.size = 1;
        constantBufferDesc.elementSize = sizeof(ConstantsCamera);
        constantBufferDesc.binding = Renderer::BufferBindingPoint::Uniform;
        constantBufferDesc.usage = Renderer::BufferUsage::TRANSFER_DST;
        constantBuffer = renderer->CreateBuffer(constantBufferDesc, nullptr);

        Renderer::BufferDescriptor lightBufferDesc;
        lightBufferDesc.size = nLights;
        lightBufferDesc.elementSize = sizeof(Light);
        lightBufferDesc.binding = Renderer::BufferBindingPoint::Uniform;
        lightBufferDesc.usage = Renderer::BufferUsage::TRANSFER_DST;
        lightBuffer = renderer->CreateBuffer(lightBufferDesc, nullptr);

        Renderer::BufferDescriptor quadBufferDesc;
        quadBufferDesc.size = quad.size();
        quadBufferDesc.binding = Renderer::BufferBindingPoint::Vertex;
        quadBufferDesc.usage = Renderer::BufferUsage::TRANSFER_DST;
        quadBufferDesc.elementSize = sizeof(Vertex);
        quadBufferDesc.type = Renderer::BufferType::Static;
        quadBuffer = renderer->CreateBuffer(std::move(quadBufferDesc), quad.data());

        Renderer::BufferDescriptor quadIdxBufferDesc;
        quadIdxBufferDesc.size = quadIdx.size();
        quadIdxBufferDesc.elementSize = sizeof(u32);
        quadIdxBufferDesc.binding = Renderer::BufferBindingPoint::Index;
        quadIdxBufferDesc.usage = Renderer::BufferUsage::TRANSFER_DST;
        quadIdxBuffer = renderer->CreateBuffer(quadIdxBufferDesc, quadIdx.data());

        return true;
    }

    void CRenderModule::Stop()
    {
        constantBuffer->~Buffer();
        lightBuffer->~Buffer();
        quadIdxBuffer->~Buffer();
        quadBuffer->~Buffer();
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

    void CRenderModule::Render()
    {
    }

    void CRenderModule::RenderInMenu()
    {
    }

    void CRenderModule::DoFrame()
    {
        // Start drawing.
        CommandBuffer cmd = renderer->BeginCommandBuffer();
        renderer->BindPipeline(&forwardPipeline, cmd);
        renderer->BeginRenderPass(&forwardRenderPass, cmd);

        // Bind frame constants.
        renderer->BindBuffer(constantBuffer, &forwardPipeline, 0, 0);
        renderer->BindBuffer(lightBuffer, &forwardPipeline, 1, 0);

        // Update constants per frame data.
        CEntity *eCamera = getEntityByName("camera");
        SASSERT(eCamera);
        TCompCamera *cCamera = eCamera->Get<TCompCamera>();
        SASSERT(cCamera);

        ConstantsCamera cameraCtes;
        cameraCtes.camera_view = cCamera->GetView();
        cameraCtes.camera_projection = cCamera->GetProjection();
        cameraCtes.camera_view_projection = cCamera->GetViewProjection();
        renderer->UpdateBuffer(constantBuffer, &cameraCtes, sizeof(ConstantsCamera), 0, cmd);

        u32 i = 0;
        GetObjectManager<TCompPointLight>()->ForEach([&](TCompPointLight *light)
                                                     {
            Light l;
            l.color     = light->color;
            l.position  = light->position;
            l.intensity = light->intensity;
            l.radius    = light->radius;

            renderer->UpdateBuffer(lightBuffer, &l, sizeof(Light), sizeof(Light) * i, cmd);
            i++; });

        // Bind constants per frame.
        renderer->BindDescriptor(cmd);

        // Draw mesh instances.
        RenderManager.RenderAll(CHandle(), DrawChannel::SOLID, cmd);

        // End drawing.
        renderer->EndRenderPass(cmd);

        // Present
        auto presentCmd = renderer->BeginCommandBuffer();
        renderer->WaitCommand(presentCmd, cmd);
        renderer->BindPipeline(&presentPipeline, presentCmd);
        renderer->BeginRenderPass(swapchain, presentCmd);

        // Bind textures
        renderer->BindAttachment(&colorAttachment, &presentPipeline, 0);
        renderer->UpdateDescriptorSet(&presentPipeline);
        renderer->BindDescriptor(presentCmd);
        renderer->BindVertexBuffer(quadBuffer, presentCmd);
        renderer->BindIndexBuffer(quadIdxBuffer, presentCmd);
        renderer->DrawIndexed(6, 0, presentCmd);
        renderer->EndRenderPass(presentCmd);

        renderer->SubmitCommandBuffers();
    }
} // Sogas
