
#include "render/pipelines/forward_pipeline.h"
#include "components/camera_component.h"
#include "components/light_point_component.h"
#include "render/render_manager.h"
#include "renderer/public/attachment.h"
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
namespace Renderer
{
ForwardPipeline::ForwardPipeline(std::shared_ptr<GPU_device> InRenderer)
: renderer(InRenderer)
{
    SASSERT(renderer != nullptr);

    // renderer->CreateShader(ShaderStageType::VERTEX, std::move(CEngine::FindFile("forward.vert.spv")), &forwardShaders[0]);
    // renderer->CreateShader(ShaderStageType::FRAGMENT, std::move(CEngine::FindFile("forward.frag.spv")), &forwardShaders[1]);
    // renderer->CreateShader(ShaderStageType::VERTEX, std::move(CEngine::FindFile("quad.vert.spv")), &presentShaders[0]);
    // renderer->CreateShader(ShaderStageType::FRAGMENT, std::move(CEngine::FindFile("quad.frag.spv")), &presentShaders[1]);

    // colorAttachment.format = Format::R8G8B8A8_SRGB;
    // colorAttachment.usage  = BindPoint::RENDER_TARGET;
    // renderer->CreateAttachment(&colorAttachment);

    // depthAttachment.format = Format::D24_UNORM_S8_UINT;
    // depthAttachment.usage  = BindPoint::DEPTH_STENCIL;
    // renderer->CreateAttachment(&depthAttachment);

    // RenderPassDescriptor rpDesc;
    // rpDesc.attachments.push_back(Attachment::RenderTarget(nullptr, &colorAttachment));
    // rpDesc.attachments.push_back(Attachment::DepthStencil(nullptr, &depthAttachment));
    // forwardRenderPass = new RenderPass(std::move(rpDesc));
    // renderer->CreateRenderPass(forwardRenderPass);

    // DepthStencilState depthState;
    // depthState.compareOp         = CompareOperation::LESS_OR_EQUAL;
    // depthState.depthTestEnabled  = true;
    // depthState.writeDepthEnabled = true;

    // PipelineDescriptor fwdDesc;
    // fwdDesc.vs                = &forwardShaders[0];
    // fwdDesc.ps                = &forwardShaders[1];
    // fwdDesc.vertexDeclaration = "PosNormalUvColor";
    // //fwdDesc.depthStencilState = &depthState;
    // renderer->CreatePipeline(&fwdDesc, &pipeline, forwardRenderPass);
    // PipelineDescriptor psoDesc;
    // psoDesc.vs                = &presentShaders[0];
    // psoDesc.ps                = &presentShaders[1];
    // psoDesc.vertexDeclaration = "PosNormalUvColor";
    //renderer->CreatePipeline(&psoDesc, &presentPipeline, swapchain->renderpass);

    // PushConstantDescriptor modelPushConstant;
    // modelPushConstant.offset = 0;
    // modelPushConstant.size   = sizeof(ConstantsMesh);
    // modelPushConstant.stage  = ShaderStageType::VERTEX;

    //     {
    // Create pipeline state
    PipelineDescriptor pipeline_creation;

    // Vertex input
    // TODO(marco): component format should be based on buffer view type
    // pipeline_creation.vertexInputState.AddVertexAttribute({0, 0, 0, VertexFormat::FLOAT3}); // position
    // pipeline_creation.vertexInputState.AddVertexStream({0, 12, VertexInputRate::PER_VERTEX});

    // pipeline_creation.vertexInputState.AddVertexAttribute({1, 1, 0, VertexFormat::FLOAT3}); // normal
    // pipeline_creation.vertexInputState.AddVertexStream({1, 12, VertexInputRate::PER_VERTEX});

    // pipeline_creation.vertexInputState.AddVertexAttribute({2, 2, 0, VertexFormat::FLOAT2}); // uvs
    // pipeline_creation.vertexInputState.AddVertexStream({2, 8, VertexInputRate::PER_VERTEX});

    // pipeline_creation.vertexInputState.AddVertexAttribute({3, 3, 0, VertexFormat::FLOAT4}); // color
    // pipeline_creation.vertexInputState.AddVertexStream({3, 16, VertexInputRate::PER_VERTEX});

    // Render pass
    pipeline_creation.render_pass = renderer->GetSwapchainOutput();
    // Depth
    pipeline_creation.depthStencilState.SetDepth(true, CompareOperation::LESS_OR_EQUAL);

    auto forward_ps = renderer->ReadShaderBinary(std::move(CEngine::FindFile("triangle.frag.spv")));
    auto forward_vs = renderer->ReadShaderBinary(std::move(CEngine::FindFile("triangle.vert.spv")));

    pipeline_creation.shaders
      .SetName("Triangle")
      .AddStage(reinterpret_cast<char*>(forward_vs.data()), static_cast<u32>(forward_vs.size()), ShaderStageType::VERTEX)
      .AddStage(reinterpret_cast<char*>(forward_ps.data()), static_cast<u32>(forward_ps.size()), ShaderStageType::FRAGMENT)
      .SetSpvInput(true);

    //DescriptorSetLayoutDescriptor descLayoutDescriptor {};
    //descLayoutDescriptor.AddBinding({DescriptorType::UNIFORM_BUFFER, 0, 1, "LocalConstants"});
    //descLayoutDescriptor.AddBinding({DescriptorType::UNIFORM_BUFFER, 1, 2, "Lights"});

    pipeline = renderer->CreatePipeline(pipeline_creation);

    //         // Shader state
    //         const char* vs_code = R"FOO(#version 450
    // layout(std140, binding = 0) uniform LocalConstants {
    //     mat4 m;
    //     mat4 vp;
    //     mat4 mInverse;
    //     vec4 eye;
    //     vec4 light;
    // };

    // layout(location=0) in vec3 position;
    // layout(location=1) in vec4 tangent;
    // layout(location=2) in vec3 normal;
    // layout(location=3) in vec2 texCoord0;

    // layout (location = 0) out vec2 vTexcoord0;
    // layout (location = 1) out vec3 vNormal;
    // layout (location = 2) out vec4 vTangent;
    // layout (location = 3) out vec4 vPosition;

    // void main() {
    //     gl_Position = vp * m * vec4(position, 1);
    //     vPosition = m * vec4(position, 1.0);
    //     vTexcoord0 = texCoord0;
    //     vNormal = mat3(mInverse) * normal;
    //     vTangent = tangent;
    // }
    // )FOO";

    //         const char* fs_code = R"FOO(#version 450
    // layout(std140, binding = 0) uniform LocalConstants {
    //     mat4 m;
    //     mat4 vp;
    //     mat4 mInverse;
    //     vec4 eye;
    //     vec4 light;
    // };

    // layout(std140, binding = 4) uniform MaterialConstant {
    //     vec4 base_color_factor;
    // };

    // layout (binding = 1) uniform sampler2D diffuseTexture;
    // layout (binding = 2) uniform sampler2D occlusionRoughnessMetalnessTexture;
    // layout (binding = 3) uniform sampler2D normalTexture;

    // layout (location = 0) in vec2 vTexcoord0;
    // layout (location = 1) in vec3 vNormal;
    // layout (location = 2) in vec4 vTangent;
    // layout (location = 3) in vec4 vPosition;

    // layout (location = 0) out vec4 frag_color;

    // #define PI 3.1415926538

    // vec3 decode_srgb( vec3 c ) {
    //     vec3 result;
    //     if ( c.r <= 0.04045) {
    //         result.r = c.r / 12.92;
    //     } else {
    //         result.r = pow( ( c.r + 0.055 ) / 1.055, 2.4 );
    //     }

    //     if ( c.g <= 0.04045) {
    //         result.g = c.g / 12.92;
    //     } else {
    //         result.g = pow( ( c.g + 0.055 ) / 1.055, 2.4 );
    //     }

    //     if ( c.b <= 0.04045) {
    //         result.b = c.b / 12.92;
    //     } else {
    //         result.b = pow( ( c.b + 0.055 ) / 1.055, 2.4 );
    //     }

    //     return clamp( result, 0.0, 1.0 );
    // }

    // vec3 encode_srgb( vec3 c ) {
    //     vec3 result;
    //     if ( c.r <= 0.0031308) {
    //         result.r = c.r * 12.92;
    //     } else {
    //         result.r = 1.055 * pow( c.r, 1.0 / 2.4 ) - 0.055;
    //     }

    //     if ( c.g <= 0.0031308) {
    //         result.g = c.g * 12.92;
    //     } else {
    //         result.g = 1.055 * pow( c.g, 1.0 / 2.4 ) - 0.055;
    //     }

    //     if ( c.b <= 0.0031308) {
    //         result.b = c.b * 12.92;
    //     } else {
    //         result.b = 1.055 * pow( c.b, 1.0 / 2.4 ) - 0.055;
    //     }

    //     return clamp( result, 0.0, 1.0 );
    // }

    // float heaviside( float v ) {
    //     if ( v > 0.0 ) return 1.0;
    //     else return 0.0;
    // }

    // void main() {
    //     // NOTE(marco): normal textures are encoded to [0, 1] but need to be mapped to [-1, 1] value
    //     vec3 bump_normal = normalize( texture(normalTexture, vTexcoord0).rgb * 2.0 - 1.0 );
    //     vec3 tangent = normalize( vTangent.xyz );
    //     vec3 bitangent = cross( normalize( vNormal ), tangent ) * vTangent.w;

    //     mat3 TBN = transpose(mat3(
    //         tangent,
    //         bitangent,
    //         normalize( vNormal )
    //     ));

    //     // vec3 V = normalize(eye.xyz - vPosition.xyz);
    //     // vec3 L = normalize(light.xyz - vPosition.xyz);
    //     // vec3 N = normalize(vNormal);
    //     // vec3 H = normalize(L + V);

    //     vec3 V = normalize( TBN * ( eye.xyz - vPosition.xyz ) );
    //     vec3 L = normalize( TBN * ( light.xyz - vPosition.xyz ) );
    //     vec3 N = bump_normal;
    //     vec3 H = normalize( L + V );

    //     vec4 rmo = texture(occlusionRoughnessMetalnessTexture, vTexcoord0);

    //     // Green channel contains roughness values
    //     float roughness = rmo.g;
    //     float alpha = pow(roughness, 2.0);

    //     // Blue channel contains metalness
    //     float metalness = rmo.b;

    //     // Red channel for occlusion value

    //     vec4 base_colour = texture(diffuseTexture, vTexcoord0) * base_color_factor;
    //     base_colour.rgb = decode_srgb( base_colour.rgb );

    //     // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#specular-brdf
    //     float NdotH = dot(N, H);
    //     float alpha_squared = alpha * alpha;
    //     float d_denom = ( NdotH * NdotH ) * ( alpha_squared - 1.0 ) + 1.0;
    //     float distribution = ( alpha_squared * heaviside( NdotH ) ) / ( PI * d_denom * d_denom );

    //     float NdotL = dot(N, L);
    //     float NdotV = dot(N, V);
    //     float HdotL = dot(H, L);
    //     float HdotV = dot(H, V);

    //     float visibility = ( heaviside( HdotL ) / ( abs( NdotL ) + sqrt( alpha_squared + ( 1.0 - alpha_squared ) * ( NdotL * NdotL ) ) ) ) * ( heaviside( HdotV ) / ( abs( NdotV ) + sqrt( alpha_squared + ( 1.0 - alpha_squared ) * ( NdotV * NdotV ) ) ) );

    //     float specular_brdf = visibility * distribution;

    //     vec3 diffuse_brdf = (1 / PI) * base_colour.rgb;

    //     // NOTE(marco): f0 in the formula notation refers to the base colour here
    //     vec3 conductor_fresnel = specular_brdf * ( base_colour.rgb + ( 1.0 - base_colour.rgb ) * pow( 1.0 - abs( HdotV ), 5 ) );

    //     // NOTE(marco): f0 in the formula notation refers to the value derived from ior = 1.5
    //     float f0 = 0.04; // pow( ( 1 - ior ) / ( 1 + ior ), 2 )
    //     float fr = f0 + ( 1 - f0 ) * pow(1 - abs( HdotV ), 5 );
    //     vec3 fresnel_mix = mix( diffuse_brdf, vec3( specular_brdf ), fr );

    //     vec3 material_colour = mix( fresnel_mix, conductor_fresnel, metalness );

    //     frag_color = vec4( encode_srgb( material_colour ), base_colour.a );
    // }
    // )FOO";

    //         pipeline_creation.shaders.set_name("Cube").add_stage(vs_code, (uint32_t)strlen(vs_code), VK_SHADER_STAGE_VERTEX_BIT).add_stage(fs_code, (uint32_t)strlen(fs_code), VK_SHADER_STAGE_FRAGMENT_BIT);

    //         // Descriptor set layout
    //         DescriptorSetLayoutCreation cube_rll_creation{};
    //         cube_rll_creation.add_binding({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants"});
    //         cube_rll_creation.add_binding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 1, "diffuseTexture"});
    //         cube_rll_creation.add_binding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1, "occlusionRoughnessMetalnessTexture"});
    //         cube_rll_creation.add_binding({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1, "normalTexture"});
    //         cube_rll_creation.add_binding({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, 1, "MaterialConstant"});
    //         // Setting it into pipeline
    //         cube_dsl = gpu.create_descriptor_set_layout(cube_rll_creation);
    //         pipeline_creation.add_descriptor_set_layout(cube_dsl);

    //         // Constant buffer
    //         BufferCreation buffer_creation;
    //         buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(UniformData)).set_name("cube_cb");
    //         cube_cb = gpu.create_buffer(buffer_creation);

    //         cube_pipeline = gpu.create_pipeline(pipeline_creation);

    //         for (u32 mesh_index = 0; mesh_index < scene.meshes_count; ++mesh_index)
    //         {
    //             MeshDraw mesh_draw{};

    //             glTF::Mesh& mesh = scene.meshes[mesh_index];

    //             for (u32 primitive_index = 0; primitive_index < mesh.primitives_count; ++primitive_index)
    //             {
    //                 glTF::MeshPrimitive& mesh_primitive = mesh.primitives[primitive_index];

    //                 i32 position_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "POSITION");
    //                 i32 tangent_accessor_index  = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TANGENT");
    //                 i32 normal_accessor_index   = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "NORMAL");
    //                 i32 texcoord_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TEXCOORD_0");

    //                 if (position_accessor_index != -1)
    //                 {
    //                     glTF::Accessor&   position_accessor    = scene.accessors[position_accessor_index];
    //                     glTF::BufferView& position_buffer_view = scene.buffer_views[position_accessor.buffer_view];
    //                     BufferResource&   position_buffer_gpu  = buffers[position_accessor.buffer_view];

    //                     mesh_draw.position_buffer = position_buffer_gpu.handle;
    //                     mesh_draw.position_offset = position_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : position_accessor.byte_offset;
    //                 }

    //                 if (tangent_accessor_index != -1)
    //                 {
    //                     glTF::Accessor&   tangent_accessor    = scene.accessors[tangent_accessor_index];
    //                     glTF::BufferView& tangent_buffer_view = scene.buffer_views[tangent_accessor.buffer_view];
    //                     BufferResource&   tangent_buffer_gpu  = buffers[tangent_accessor.buffer_view];

    //                     mesh_draw.tangent_buffer = tangent_buffer_gpu.handle;
    //                     mesh_draw.tangent_offset = tangent_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : tangent_accessor.byte_offset;
    //                 }

    //                 if (normal_accessor_index != -1)
    //                 {
    //                     glTF::Accessor&   normal_accessor    = scene.accessors[normal_accessor_index];
    //                     glTF::BufferView& normal_buffer_view = scene.buffer_views[normal_accessor.buffer_view];
    //                     BufferResource&   normal_buffer_gpu  = buffers[normal_accessor.buffer_view];

    //                     mesh_draw.normal_buffer = normal_buffer_gpu.handle;
    //                     mesh_draw.normal_offset = normal_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : normal_accessor.byte_offset;
    //                 }

    //                 if (texcoord_accessor_index != -1)
    //                 {
    //                     glTF::Accessor&   texcoord_accessor    = scene.accessors[texcoord_accessor_index];
    //                     glTF::BufferView& texcoord_buffer_view = scene.buffer_views[texcoord_accessor.buffer_view];
    //                     BufferResource&   texcoord_buffer_gpu  = buffers[texcoord_accessor.buffer_view];

    //                     mesh_draw.texcoord_buffer = texcoord_buffer_gpu.handle;
    //                     mesh_draw.texcoord_offset = texcoord_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : texcoord_accessor.byte_offset;
    //                 }

    //                 glTF::Accessor&   indices_accessor    = scene.accessors[mesh_primitive.indices];
    //                 glTF::BufferView& indices_buffer_view = scene.buffer_views[indices_accessor.buffer_view];
    //                 BufferResource&   indices_buffer_gpu  = buffers[indices_accessor.buffer_view];
    //                 mesh_draw.index_buffer                = indices_buffer_gpu.handle;
    //                 mesh_draw.index_offset                = indices_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : indices_accessor.byte_offset;

    //                 glTF::Material& material = scene.materials[mesh_primitive.material];

    //                 // Descriptor Set
    //                 DescriptorSetCreation ds_creation{};
    //                 ds_creation.set_layout(cube_dsl).buffer(cube_cb, 0);

    //                 // NOTE(marco): for now we expect all three textures to be defined. In the next chapter
    //                 // we'll relax this constraint thanks to bindless rendering!

    //                 if (material.pbr_metallic_roughness != nullptr)
    //                 {
    //                     if (material.pbr_metallic_roughness->base_color_factor_count != 0)
    //                     {
    //                         RASSERT(material.pbr_metallic_roughness->base_color_factor_count == 4);

    //                         mesh_draw.material_data.base_color_factor = {
    //                           material.pbr_metallic_roughness->base_color_factor[0],
    //                           material.pbr_metallic_roughness->base_color_factor[1],
    //                           material.pbr_metallic_roughness->base_color_factor[2],
    //                           material.pbr_metallic_roughness->base_color_factor[3],
    //                         };
    //                     }
    //                     else
    //                     {
    //                         mesh_draw.material_data.base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    //                     }

    //                     if (material.pbr_metallic_roughness->base_color_texture != nullptr)
    //                     {
    //                         glTF::Texture&   diffuse_texture     = scene.textures[material.pbr_metallic_roughness->base_color_texture->index];
    //                         TextureResource& diffuse_texture_gpu = images[diffuse_texture.source];
    //                         SamplerResource& diffuse_sampler_gpu = samplers[diffuse_texture.sampler];

    //                         ds_creation.texture_sampler(diffuse_texture_gpu.handle, diffuse_sampler_gpu.handle, 1);
    //                     }
    //                     else
    //                     {
    //                         continue;
    //                     }

    //                     if (material.pbr_metallic_roughness->metallic_roughness_texture != nullptr)
    //                     {
    //                         glTF::Texture&   roughness_texture     = scene.textures[material.pbr_metallic_roughness->metallic_roughness_texture->index];
    //                         TextureResource& roughness_texture_gpu = images[roughness_texture.source];
    //                         SamplerResource& roughness_sampler_gpu = samplers[roughness_texture.sampler];

    //                         ds_creation.texture_sampler(roughness_texture_gpu.handle, roughness_sampler_gpu.handle, 2);
    //                     }
    //                     else if (material.occlusion_texture != nullptr)
    //                     {
    //                         glTF::Texture& occlusion_texture = scene.textures[material.occlusion_texture->index];

    //                         TextureResource& occlusion_texture_gpu = images[occlusion_texture.source];
    //                         SamplerResource& occlusion_sampler_gpu = samplers[occlusion_texture.sampler];

    //                         ds_creation.texture_sampler(occlusion_texture_gpu.handle, occlusion_sampler_gpu.handle, 2);
    //                     }
    //                     else
    //                     {
    //                         continue;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     continue;
    //                 }

    //                 if (material.normal_texture != nullptr)
    //                 {
    //                     glTF::Texture&   normal_texture     = scene.textures[material.normal_texture->index];
    //                     TextureResource& normal_texture_gpu = images[normal_texture.source];
    //                     SamplerResource& normal_sampler_gpu = samplers[normal_texture.sampler];

    //                     ds_creation.texture_sampler(normal_texture_gpu.handle, normal_sampler_gpu.handle, 3);
    //                 }
    //                 else
    //                 {
    //                     continue;
    //                 }

    //                 buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(MaterialData)).set_name("material");
    //                 mesh_draw.material_buffer = gpu.create_buffer(buffer_creation);
    //                 ds_creation.buffer(mesh_draw.material_buffer, 4);

    //                 mesh_draw.count = indices_accessor.count;

    //                 mesh_draw.descriptor_set = gpu.create_descriptor_set(ds_creation);

    //                 mesh_draws.push(mesh_draw);
    //             }
    //         }

    //         rx = 0.0f;
    //         ry = 0.0f;
    //     }
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
    cmd->draw(0, 3, 0, 1);

    renderer->QueueCommandBuffer(cmd);

    renderer->Present();

    // Bind constants per frame.
    // Draw mesh instances.
    // CommandBuffer* cmd;
    // RenderManager.RenderAll(CHandle(), DrawChannel::SOLID, *cmd);
}

void ForwardPipeline::destroy()
{
}
} // namespace Renderer
} // namespace Sogas
