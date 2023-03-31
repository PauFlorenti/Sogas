
#include "render/pipelines/forward_pipeline.h"
#include "components/camera_component.h"
#include "components/light_point_component.h"
#include "render/render_manager.h"
#include "renderer/public/buffer.h"
#include "renderer/public/render_device.h"
#include "renderer/public/render_types.h"
#include "renderer/public/renderpass.h"

std::vector<Sogas::Renderer::VertexLayout> quad = {
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
    f32       intensity;
    glm::vec3 position;
    f32       radius;
};

Light lights[10];

struct ConstantsMesh
{
    glm::mat4 model;
    glm::vec4 color;
};

namespace Sogas
{

using namespace Renderer;

ForwardPipeline::ForwardPipeline(std::shared_ptr<Renderer::GPU_device> InRenderer)
: renderer(InRenderer)
{
    SASSERT(renderer != nullptr);

    // Create pipeline state
    PipelineDescriptor pipeline_creation;

    // Vertex input
    // TODO(marco): component format should be based on buffer view type
    pipeline_creation.vertexInputState.AddVertexAttribute({0, 0, 0, VertexFormat::FLOAT3}); // position
    pipeline_creation.vertexInputState.AddVertexStream({0, 12, VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({1, 1, 0, VertexFormat::FLOAT3}); // normal
    pipeline_creation.vertexInputState.AddVertexStream({1, 12, VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({2, 2, 0, VertexFormat::FLOAT2}); // uvs
    pipeline_creation.vertexInputState.AddVertexStream({2, 8, VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({3, 3, 0, VertexFormat::FLOAT4}); // color
    pipeline_creation.vertexInputState.AddVertexStream({3, 16, VertexInputRate::PER_VERTEX});

    // Render pass
    pipeline_creation.render_pass = renderer->GetSwapchainOutput();
    // Depth
    pipeline_creation.depthStencilState.SetDepth(true, CompareOperation::LESS_OR_EQUAL);

    auto forward_ps = renderer->ReadShaderBinary(std::move(CEngine::FindFile("forward.frag.spv")));
    auto forward_vs = renderer->ReadShaderBinary(std::move(CEngine::FindFile("forward.vert.spv")));

    pipeline_creation.shaders
      .SetName("Triangle")
      .AddStage(reinterpret_cast<char*>(forward_vs.data()), static_cast<u32>(forward_vs.size()), ShaderStageType::VERTEX)
      .AddStage(reinterpret_cast<char*>(forward_ps.data()), static_cast<u32>(forward_ps.size()), ShaderStageType::FRAGMENT)
      .SetSpvInput(true);

    DescriptorSetLayoutDescriptor descLayoutDescriptor{};
    descLayoutDescriptor.AddBinding({DescriptorType::UNIFORM_BUFFER, 0, 1, "UniformBufferObject"});
    descLayoutDescriptor.AddBinding({DescriptorType::UNIFORM_BUFFER, 1, 1, "MeshObject"});
    descLayoutDescriptor.AddBinding({DescriptorType::UNIFORM_BUFFER, 2, 2, "Lights"});

    descriptorLayout = renderer->CreateDescriptorSetLayout(std::move(descLayoutDescriptor));
    pipeline_creation.AddDescriptorSetLayout(descriptorLayout);

    pipeline = renderer->CreatePipeline(pipeline_creation);

    BufferDescriptor ubo_desc;
    ubo_desc.reset().set(BufferUsage::UNIFORM, BufferBindingPoint::Uniform, sizeof(ConstantsCamera)).setName("UniformBufferObject");

    camera_buffer = renderer->CreateBuffer(std::move(ubo_desc));

    BufferDescriptor mesh_desc;
    mesh_desc.reset().set(BufferUsage::UNIFORM, BufferBindingPoint::Uniform, sizeof(ConstantsMesh)).setName("MeshObject");

    mesh_buffer = renderer->CreateBuffer(std::move(mesh_desc));

    BufferDescriptor light_desc;
    light_desc.reset().set(BufferUsage::UNIFORM, BufferBindingPoint::Uniform, sizeof(Light)).setName("Lights");

    light_buffer = renderer->CreateBuffer(std::move(light_desc));
}

void ForwardPipeline::update_constants()
{
}

void ForwardPipeline::render()
{
    // Update constants per frame data.
    CEntity* eCamera = getEntityByName("camera");
    SASSERT(eCamera);
    TCompCamera* cCamera = eCamera->Get<TCompCamera>();
    SASSERT(cCamera);

    ConstantsCamera cameraCtes;
    cameraCtes.camera_view            = cCamera->GetView();
    cameraCtes.camera_projection      = cCamera->GetProjection();
    cameraCtes.camera_view_projection = cCamera->GetViewProjection();

    u32 i = 0;
    GetObjectManager<TCompPointLight>()->ForEach(
      [&](TCompPointLight* light)
      {
          Light l;
          l.color     = light->color;
          l.position  = light->position;
          l.intensity = light->intensity;
          l.radius    = light->radius;

          i++;
      });

    renderer->BeginFrame();

    CommandBuffer* cmd = renderer->GetCommandBuffer(true);

    // TODO we may want to send custom viewport and scissors.
    cmd->set_viewport();
    cmd->set_scissors();

    cmd->bind_pass(renderer->GetSwapchainRenderpass());
    cmd->bind_pipeline(pipeline);
    //cmd->draw(0, 3, 0, 1);

    renderer->QueueCommandBuffer(cmd);

    renderer->Present();

    // Bind constants per frame.
    // Draw mesh instances.
    // CommandBuffer* cmd;
    // RenderManager.RenderAll(CHandle(), DrawChannel::SOLID, *cmd);
}

void ForwardPipeline::destroy()
{
    renderer->DestroyBuffer(camera_buffer);
    renderer->DestroyBuffer(mesh_buffer);
    renderer->DestroyBuffer(light_buffer);
    renderer->DestroyDescriptorSetLayout(descriptorLayout);
    renderer->DestroyPipeline(pipeline);
}
} // namespace Sogas
