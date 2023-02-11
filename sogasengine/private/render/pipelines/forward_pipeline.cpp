
#include "components/camera_component.h"
#include "components/light_point_component.h"
#include "render/render_manager.h"
#include "renderer/public/buffer.h"
#include "render/pipelines/forward_pipeline.h"
#include "renderer/public/render_device.h"
#include "renderer/public/render_types.h"

std::vector<Sogas::Vertex> quad = {
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

Light lights[10];

struct ConstantsMesh
{
    glm::mat4 model;
    glm::vec4 color;
};

namespace Sogas
{
    namespace Renderer
    {
        ForwardPipeline::ForwardPipeline(std::shared_ptr<GPU_device> InRenderer, std::shared_ptr<Swapchain> swapchain)
        : renderer(InRenderer)
        {
            SASSERT(renderer != nullptr);
            SASSERT(swapchain.get() != nullptr);

            renderer->CreateShader(ShaderStage::VERTEX, std::move(CEngine::FindFile("forward.vert.spv")), &forwardShaders[0]);
            renderer->CreateShader(ShaderStage::FRAGMENT, std::move(CEngine::FindFile("forward.frag.spv")), &forwardShaders[1]);
            renderer->CreateShader(ShaderStage::VERTEX, std::move(CEngine::FindFile("quad.vert.spv")), &presentShaders[0]);
            renderer->CreateShader(ShaderStage::FRAGMENT, std::move(CEngine::FindFile("quad.frag.spv")), &presentShaders[1]);

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
            renderer->CreatePipeline(&fwdDesc, &pipeline, &forwardRenderPass);

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
            quadBufferDesc.elementSize = sizeof(Sogas::Vertex);
            quadBufferDesc.type = Renderer::BufferType::Static;
            quadBuffer = renderer->CreateBuffer(std::move(quadBufferDesc), quad.data());

            Renderer::BufferDescriptor quadIdxBufferDesc;
            quadIdxBufferDesc.size = quadIdx.size();
            quadIdxBufferDesc.elementSize = sizeof(u32);
            quadIdxBufferDesc.binding = Renderer::BufferBindingPoint::Index;
            quadIdxBufferDesc.usage = Renderer::BufferUsage::TRANSFER_DST;
            quadIdxBuffer = renderer->CreateBuffer(std::move(quadIdxBufferDesc), quadIdx.data());
        }

        void ForwardPipeline::update_constants()
        {
        }

        void ForwardPipeline::render(std::shared_ptr<Swapchain> swapchain)
        {
            // Start drawing.
            CommandBuffer cmd = renderer->BeginCommandBuffer();
            renderer->BindPipeline(&pipeline, cmd);
            renderer->BeginRenderPass(&forwardRenderPass, cmd);

            // Bind frame constants.
            renderer->BindBuffer(constantBuffer, &pipeline, 0, 0);
            renderer->BindBuffer(lightBuffer, &pipeline, 1, 0);

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

        void ForwardPipeline::present()
        {
        }

        void ForwardPipeline::destroy()
        {
            constantBuffer->Release();
            lightBuffer->Release();
            quadIdxBuffer->Release();
            quadBuffer->Release();
        }
    } // namespace Renderer
} // namespace Sogas
