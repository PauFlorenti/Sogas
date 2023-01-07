
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
    Pipeline pipeline;
    Shader shaders[2];
    DescriptorSet descriptorSet;
    GPUBuffer constantBuffer;
    GPUBuffer lightBuffer;

    const u32 nLights = 10;

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

        swapchain = std::make_shared<Swapchain>();
        SwapchainDescriptor desc;
        desc.format = Format::R32G32B32A32_SFLOAT;
        desc.width  = width;
        desc.height = height;
        renderer->CreateSwapchain(desc, swapchain.get());

        // Init global buffers
        renderer->CreateShader(ShaderStage::VERTEX, "forward.vert.spv", &shaders[0]);
        renderer->CreateShader(ShaderStage::FRAGMENT, "forward.frag.spv", &shaders[1]);

        PushConstantDescriptor modelPushConstant;
        modelPushConstant.offset        = 0;
        modelPushConstant.size          = sizeof(ConstantsMesh);
        modelPushConstant.stage         = ShaderStage::VERTEX;

        GPUBufferDescriptor constantBufferDesc;
        constantBufferDesc.bindPoint    = BindPoint::UNIFORM;
        constantBufferDesc.size         = sizeof(ConstantsCamera);
        constantBufferDesc.usage        = Usage::UPLOAD;
        renderer->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

        GPUBufferDescriptor lightBufferDesc;
        lightBufferDesc.size            = sizeof(Light) * nLights;
        lightBufferDesc.bindPoint       = BindPoint::UNIFORM;
        lightBufferDesc.usage           = Usage::UPLOAD;
        renderer->CreateBuffer(&lightBufferDesc, nullptr, &lightBuffer);

        DescriptorSetDescriptor constDescriptorSet;
        constDescriptorSet.binding      = 0;
        constDescriptorSet.stage        = ShaderStage::VERTEX;
        constDescriptorSet.uniformType  = UniformType::UNIFORM;
        constDescriptorSet.size         = static_cast<u32>(sizeof(ConstantsCamera));
        constDescriptorSet.offset       = 0;
        constDescriptorSet.buffer       = &constantBuffer;

        DescriptorSetDescriptor lightDescriptor;
        lightDescriptor.binding     = 1;
        lightDescriptor.stage       = ShaderStage::FRAGMENT;
        lightDescriptor.uniformType = UniformType::UNIFORM;
        lightDescriptor.size        = static_cast<u32>(sizeof(Light)) * nLights;
        lightDescriptor.offset      = 0;
        lightDescriptor.buffer      = &lightBuffer;

        PipelineDescriptor psoDesc;
        psoDesc.vs = &shaders[0];
        psoDesc.ps = &shaders[1];
        psoDesc.vertexDeclaration = "PosNormalUvColor";
        psoDesc.pushConstantDesc.push_back(modelPushConstant);
        psoDesc.descriptorSetDesc.push_back(constDescriptorSet);
        psoDesc.descriptorSetDesc.push_back(lightDescriptor);

        renderer->CreatePipeline(&psoDesc, &pipeline, &swapchain->renderpass);
        renderer->CreateDescriptorSet(&descriptorSet, &pipeline);

        renderer->UpdateDescriptorSet(&descriptorSet, psoDesc.descriptorSetDesc);

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
        renderer->BindPipeline(&pipeline, cmd);
        renderer->BeginRenderPass(swapchain.get(), cmd);

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
        renderer->BindDescriptor(&descriptorSet, cmd);

        // Draw mesh instances.
       RenderManager.RenderAll(CHandle(), DrawChannel::SOLID, cmd);

        // End drawing.
        renderer->EndRenderPass(cmd);
        renderer->SubmitCommandBuffers();
    }
} // Sogas
