#pragma once

namespace Sogas
{
namespace Renderer
{

typedef u32 ResourceHandle;

// Resource Handles
// clang-format off
struct BufferHandle { ResourceHandle index; };
struct TextureHandle { ResourceHandle index; };
struct ShaderStateHandle { ResourceHandle index; };
struct SamplerHandle { ResourceHandle index; };
struct DescriptorSetHandle { ResourceHandle index; };
struct DescriptorSetLayoutHandle { ResourceHandle index; };
struct RenderPassHandle { ResourceHandle index; };
struct PipelineHandle { ResourceHandle index; };

// Invalid Handles
static BufferHandle                 InvalidBuffer { INVALID_ID };
static TextureHandle                InvalidTexture { INVALID_ID };
static ShaderStateHandle            InvalidShaderStage { INVALID_ID };
static SamplerHandle                InvalidSampler { INVALID_ID };
static DescriptorSetHandle          InvalidDescriptorSet { INVALID_ID };
static DescriptorSetLayoutHandle    InvalidDescriptorSetLayout { INVALID_ID };
static RenderPassHandle             InvalidRenderPass { INVALID_ID };
static PipelineHandle               InvalidPipeline { INVALID_ID };
// clang-format on

static const u8 MAX_SHADER_STAGES          = 5;
static const u8 MAX_IMAGE_OUTPUTS          = 8;
static const u8 MAX_DESCRIPTOR_SET_LAYOUTS = 8;
static const u8 MAX_DESCRIPTOR_PER_SET     = 16;
static const u8 MAX_VERTEX_STREAMS         = 16;
static const u8 MAX_VERTEX_ATTRIBUTE       = 16;

} // namespace Renderer
} // namespace Sogas