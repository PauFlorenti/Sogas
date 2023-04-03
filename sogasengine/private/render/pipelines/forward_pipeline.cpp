
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

Sogas::Renderer::BufferHandle buffer_quad;
Sogas::Renderer::BufferHandle buffer_quad_index;

std::vector<glm::vec3> quad_positions = { {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f} };
std::vector<glm::vec3> quad_normals = { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
std::vector<glm::vec2> quad_uvs = { {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f} };
std::vector<glm::vec4> quad_colors = { {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };

struct MeshTest
{
    Sogas::Renderer::BufferHandle position_buffer;
    Sogas::Renderer::BufferHandle normal_buffer;
    Sogas::Renderer::BufferHandle uvs_buffer;
    Sogas::Renderer::BufferHandle color_buffer;
};

MeshTest mesh;

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
    pipeline_creation.vertexInputState.AddVertexAttribute({0, 0, 0, VertexFormat::FLOAT3}); // position
    pipeline_creation.vertexInputState.AddVertexStream({0, sizeof(glm::vec3), VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({1, 1, 0, VertexFormat::FLOAT3}); // normal
    pipeline_creation.vertexInputState.AddVertexStream({1, sizeof(glm::vec3), VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({2, 2, 0, VertexFormat::FLOAT2}); // uvs
    pipeline_creation.vertexInputState.AddVertexStream({2, sizeof(glm::vec2), VertexInputRate::PER_VERTEX});

    pipeline_creation.vertexInputState.AddVertexAttribute({3, 3, 0, VertexFormat::FLOAT4}); // color
    pipeline_creation.vertexInputState.AddVertexStream({3, sizeof(glm::vec4), VertexInputRate::PER_VERTEX});

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
    ubo_desc.reset().set(BufferUsage::UNIFORM, BufferType::Static, BufferBindingPoint::Uniform, sizeof(ConstantsCamera)).setName("UniformBufferObject");

    camera_buffer = renderer->CreateBuffer(std::move(ubo_desc));

    BufferDescriptor mesh_desc;
    mesh_desc.reset().set(BufferUsage::UNIFORM, BufferType::Static, BufferBindingPoint::Uniform, sizeof(ConstantsMesh)).setName("MeshObject");

    mesh_buffer = renderer->CreateBuffer(std::move(mesh_desc));

    BufferDescriptor light_desc;
    light_desc.reset().set(BufferUsage::UNIFORM, BufferType::Static, BufferBindingPoint::Uniform, sizeof(Light) * 2).setName("Lights");

    light_buffer = renderer->CreateBuffer(std::move(light_desc));

    DescriptorSetDescriptor descriptorSet_desc;
    descriptorSet_desc.SetLayout(descriptorLayout).Buffer(camera_buffer, 0).Buffer(mesh_buffer, 1).Buffer(light_buffer, 2);

    descriptorSet = renderer->CreateDescriptorSet(std::move(descriptorSet_desc));

    BufferDescriptor quad_descriptor;
    quad_descriptor.reset().set(BufferUsage::VERTEX, BufferType::Static, BufferBindingPoint::Vertex, static_cast<u32>(quad.size() * sizeof(VertexLayout))).setData(quad.data());

    BufferDescriptor quad_positions_descriptor;
    quad_positions_descriptor.reset().set(BufferUsage::VERTEX, BufferType::Static, BufferBindingPoint::Vertex, static_cast<u32>(quad_positions.size() * sizeof(glm::vec3))).setData(quad_positions.data());

    BufferDescriptor quad_normals_descriptor;
    quad_normals_descriptor.reset().set(BufferUsage::VERTEX, BufferType::Static, BufferBindingPoint::Vertex, static_cast<u32>(quad_normals.size() * sizeof(glm::vec3))).setData(quad_normals.data());

    BufferDescriptor quad_uvs_descriptor;
    quad_uvs_descriptor.reset().set(BufferUsage::VERTEX, BufferType::Static, BufferBindingPoint::Vertex, static_cast<u32>(quad_uvs.size() * sizeof(glm::vec2))).setData(quad_uvs.data());

    BufferDescriptor quad_colors_descriptor;
    quad_colors_descriptor.reset().set(BufferUsage::VERTEX, BufferType::Static, BufferBindingPoint::Vertex, static_cast<u32>(quad_colors.size() * sizeof(glm::vec2))).setData(quad_colors.data());

    BufferDescriptor index_quad_descriptor;
    index_quad_descriptor.reset().set(BufferUsage::INDEX, BufferType::Static, BufferBindingPoint::Index, static_cast<u32>(quadIdx.size() * sizeof(u32))).setData(quadIdx.data());

    buffer_quad = renderer->CreateBuffer(std::move(quad_descriptor));

    mesh.position_buffer = renderer->CreateBuffer(std::move(quad_positions_descriptor));
    mesh.normal_buffer = renderer->CreateBuffer(std::move(quad_normals_descriptor));
    mesh.uvs_buffer = renderer->CreateBuffer(std::move(quad_uvs_descriptor));
    mesh.color_buffer = renderer->CreateBuffer(std::move(quad_colors_descriptor));
    buffer_quad_index = renderer->CreateBuffer(std::move(index_quad_descriptor));
}

void ForwardPipeline::update_constants()
{
}

void ForwardPipeline::render()
{
    renderer->BeginFrame();

    CommandBuffer* cmd = renderer->GetCommandBuffer(true);

    // TODO we may want to send custom viewport and scissors.
    cmd->set_viewport();
    cmd->set_scissors();

    cmd->bind_pass(renderer->GetSwapchainRenderpass());
    cmd->bind_pipeline(pipeline);

    // Update constants per frame data.
    CEntity* eCamera = getEntityByName("camera");
    SASSERT(eCamera);
    TCompCamera* cCamera = eCamera->Get<TCompCamera>();
    SASSERT(cCamera);

    auto camera_data = renderer->MapBuffer(camera_buffer, sizeof(ConstantsCamera));
    ConstantsCamera camera_ctes;
    camera_ctes.camera_view                    = cCamera->GetView();
    camera_ctes.camera_projection              = cCamera->GetProjection();
    camera_ctes.camera_view_projection         = cCamera->GetViewProjection();
    camera_ctes.camera_inverse_view_projection = glm::inverse(cCamera->GetViewProjection());

    memcpy(camera_data, &camera_ctes, sizeof(ConstantsCamera));
    renderer->UnmapBuffer(camera_buffer);

    auto mesh_data = renderer->MapBuffer(mesh_buffer, sizeof(ConstantsMesh));

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
    ConstantsMesh mesh_ctes;
    mesh_ctes.model = model;
    mesh_ctes.color = glm::vec4(1.0f);

    memcpy(mesh_data, &mesh_ctes, sizeof(ConstantsMesh));
    renderer->UnmapBuffer(mesh_buffer);

    u32 i = 0;
    GetObjectManager<TCompPointLight>()->ForEach(
        [&](TCompPointLight* light)
        {
            Light l;
            l.color = light->color;
            l.position = light->position;
            l.intensity = light->intensity;
            l.radius = light->radius;

            auto light_data = renderer->MapBuffer(light_buffer, sizeof(Light), sizeof(Light) * i );

            memcpy(light_data, &l, sizeof(Light));
            renderer->UnmapBuffer(light_buffer);

            ++i;
        });

    cmd->bind_vertex_buffer(mesh.position_buffer, 0, 0);
    cmd->bind_vertex_buffer(mesh.normal_buffer, 1, 0);
    cmd->bind_vertex_buffer(mesh.uvs_buffer, 2, 0);
    cmd->bind_vertex_buffer(mesh.color_buffer, 3, 0);
    cmd->bind_index_buffer(buffer_quad_index, 0);
    cmd->bind_descriptor_set(descriptorSet, nullptr, 0);

    cmd->draw_indexed(6, 1, 0, 0, 0);

    renderer->QueueCommandBuffer(cmd);

    renderer->Present();

    // Bind constants per frame.
    // Draw mesh instances.
    // CommandBuffer* cmd;
    // RenderManager.RenderAll(CHandle(), DrawChannel::SOLID, *cmd);
}

void ForwardPipeline::destroy()
{
    renderer->DestroyBuffer(buffer_quad);
    renderer->DestroyBuffer(buffer_quad_index);

    renderer->DestroyBuffer(mesh.position_buffer);
    renderer->DestroyBuffer(mesh.normal_buffer);
    renderer->DestroyBuffer(mesh.uvs_buffer);
    renderer->DestroyBuffer(mesh.color_buffer);

    renderer->DestroyBuffer(camera_buffer);
    renderer->DestroyBuffer(mesh_buffer);
    renderer->DestroyBuffer(light_buffer);
    renderer->DestroyDescriptorSet(descriptorSet);
    renderer->DestroyDescriptorSetLayout(descriptorLayout);
    renderer->DestroyPipeline(pipeline);
}
} // namespace Sogas
