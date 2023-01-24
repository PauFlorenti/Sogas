
#include "render/module_render.h"
#include "render/render_device.h"
#include "render/render_manager.h"

// ! TEMP
#include "application.h"
#include "components/camera_component.h"
#include "components/light_point_component.h"
#include "resources/mesh.h"

namespace Sogas
{
    Pipeline    forwardPipeline; // Forward pipeline
    Pipeline    presentPipeline;
    RenderPass  forwardRenderPass;
    Shader      forwardShaders[2]; // 0 vs, 1 ps
    Shader      presentShaders[2];
    GPUBuffer   constantBuffer;
    GPUBuffer   lightBuffer;
    GPUBuffer   quadBuffer;
    GPUBuffer   quadIdxBuffer;

    std::shared_ptr<Texture>     colorBuffer;
    //Texture     depthBuffer;
    AttachmentFramebuffer colorAttachment;
    const u32   nLights = 10;

    std::vector<Vertex> quad = {
        {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
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
        glm::vec3   color;
        f32         intensity;
        glm::vec3   position;
        f32         radius;
    };

    Light lights[nLights];

    struct ConstantsMesh
    {
        glm::mat4 model;
        glm::vec4 color;
    };

    bool CRenderModule::Start () 
    {
        // Start ImGui

        // Start selected renderer. Vulkan only at the moment and by default.
        renderer = GPU_device::create(GraphicsAPI::Vulkan, nullptr);
        renderer->Init();

        i32 width, height;
        CApplication::Get()->GetWindowSize(&width, &height);

        // TODO make swapchain unique ??
        swapchain = std::make_shared<Swapchain>();
        SwapchainDescriptor desc;
        desc.format = Format::R32G32B32A32_SFLOAT;
        desc.width  = width;
        desc.height = height;
        renderer->CreateSwapchain(desc, swapchain);

        // Create shaders
        renderer->CreateShader(ShaderStage::VERTEX, "forward.vert.spv", &forwardShaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "forward.frag.spv", &forwardShaders[1]);
        renderer->CreateShader(ShaderStage::VERTEX, "quad.vert.spv", &presentShaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "quad.frag.spv", &presentShaders[1]);

        /*
        TextureDescriptor colorDesc;
        colorDesc.bindPoint     = BindPoint::RENDER_TARGET;
        colorDesc.textureType   = TextureDescriptor::TEXTURE_TYPE_2D;
        colorDesc.usage         = Usage::UPLOAD;
        colorDesc.format        = Format::R8G8B8A8_SRGB;
        colorDesc.width         = width;
        colorDesc.height        = height;
        colorDesc.depth         = 1;

        TextureDescriptor depthDesc;
        depthDesc.bindPoint     = BindPoint::DEPTH_STENCIL;
        depthDesc.textureType   = TextureDescriptor::TEXTURE_TYPE_2D;
        depthDesc.usage         = Usage::UPLOAD;
        depthDesc.format        = Format::D24_UNORM_S8_UINT;
        depthDesc.width         = width;
        depthDesc.height        = height;
        depthDesc.depth         = 1;
        */
        //renderer->CreateTexture(&colorDesc, nullptr, &colorBuffer);
        //renderer->CreateTexture(&depthDesc, nullptr, &depthBuffer);
        // 
        colorAttachment.format = Format::R8G8B8A8_SRGB;
        colorAttachment.usage = BindPoint::RENDER_TARGET;
        renderer->CreateAttachment(&colorAttachment);

        RenderPassDescriptor rpDesc;
        rpDesc.attachments.push_back(Attachment::RenderTarget(nullptr, &colorAttachment));
        //rpDesc.attachments.push_back(Attachment::DepthStencil(&depthBuffer));
        renderer->CreateRenderPass(&rpDesc, &forwardRenderPass);
        
        PipelineDescriptor fwdDesc;
        fwdDesc.vs = &forwardShaders[0];
        fwdDesc.ps = &forwardShaders[1];
        fwdDesc.vertexDeclaration = "PosNormalUvColor";
        renderer->CreatePipeline(&fwdDesc, &forwardPipeline, &forwardRenderPass);

        PipelineDescriptor psoDesc;
        psoDesc.vs = &presentShaders[0];
        psoDesc.ps = &presentShaders[1];
        psoDesc.vertexDeclaration = "PosNormalUvColor";
        renderer->CreatePipeline(&psoDesc, &presentPipeline, &swapchain->renderpass);

        PushConstantDescriptor modelPushConstant;
        modelPushConstant.offset        = 0;
        modelPushConstant.size          = sizeof(ConstantsMesh);
        modelPushConstant.stage         = ShaderStage::VERTEX;

        GPUBufferDescriptor constantBufferDesc;
        constantBufferDesc.size         = sizeof(ConstantsCamera);
        constantBufferDesc.bindPoint    = BindPoint::UNIFORM;
        constantBufferDesc.usage        = Usage::UPLOAD;
        renderer->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

        GPUBufferDescriptor lightBufferDesc;
        lightBufferDesc.size            = sizeof(Light) * nLights;
        lightBufferDesc.bindPoint       = BindPoint::UNIFORM;
        lightBufferDesc.usage           = Usage::UPLOAD;
        renderer->CreateBuffer(&lightBufferDesc, nullptr, &lightBuffer);

        GPUBufferDescriptor quadBufferDesc;
        quadBufferDesc.size         = sizeof(Vertex) * quad.size();
        quadBufferDesc.bindPoint    = BindPoint::VERTEX;
        quadBufferDesc.usage        = Usage::UPLOAD;
        renderer->CreateBuffer(&quadBufferDesc, quad.data(), &quadBuffer);

        GPUBufferDescriptor quadIdxBufferDesc;
        quadIdxBufferDesc.size = sizeof(u32) * quadIdx.size();
        quadIdxBufferDesc.bindPoint = BindPoint::INDEX;
        quadIdxBufferDesc.usage = Usage::UPLOAD;
        renderer->CreateBuffer(&quadIdxBufferDesc, quadIdx.data(), &quadIdxBuffer);

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
        // Start drawing.
        CommandBuffer cmd = renderer->BeginCommandBuffer();
        renderer->BindPipeline(&forwardPipeline, cmd);
        renderer->BeginRenderPass(&forwardRenderPass, cmd);

        //renderer->WaitCommand(cmd, swapchain.get());

        // Bind frame constants.
        renderer->BindBuffer(&constantBuffer, &forwardPipeline, 0, 0);
        renderer->BindBuffer(&lightBuffer, &forwardPipeline, 1, 0);

        // Update constants per frame data.
        CEntity* eCamera = getEntityByName("camera");
        SASSERT(eCamera);
        TCompCamera* cCamera = eCamera->Get<TCompCamera>();
        SASSERT(cCamera);

        ConstantsCamera cameraCtes;
        cameraCtes.camera_view              = cCamera->GetView();
        cameraCtes.camera_projection        = cCamera->GetProjection();
        cameraCtes.camera_view_projection   = cCamera->GetViewProjection();
        renderer->UpdateBuffer(&constantBuffer, &cameraCtes, sizeof(ConstantsCamera), 0, cmd);

        u32 i = 0;
        GetObjectManager<TCompPointLight>()->ForEach([&](TCompPointLight* light)
        {
            Light l;
            l.color     = light->color;
            l.position  = light->position;
            l.intensity = light->intensity;
            l.radius    = light->radius;

            renderer->UpdateBuffer(&lightBuffer, &l, sizeof(Light), sizeof(Light) * i, cmd);
            i++;
        });

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
        //renderer->BindTexture(&colorBuffer, &presentPipeline, 0, 0);
        renderer->BindAttachment(&colorAttachment, &presentPipeline, 0);
        renderer->UpdateDescriptorSet(&presentPipeline);
        renderer->BindDescriptor(presentCmd);
        renderer->BindVertexBuffer(&quadBuffer, presentCmd);
        renderer->BindIndexBuffer(&quadIdxBuffer, presentCmd);
        renderer->DrawIndexed(6, 0, presentCmd);
        renderer->EndRenderPass(presentCmd);
        /*
        */

        renderer->SubmitCommandBuffers();
    }
} // Sogas
