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
static ShaderStateHandle            InvalidShaderState { INVALID_ID };
static SamplerHandle                InvalidSampler { INVALID_ID };
static DescriptorSetHandle          InvalidDescriptorSet { INVALID_ID };
static DescriptorSetLayoutHandle    InvalidDescriptorSetLayout { INVALID_ID };
static RenderPassHandle             InvalidRenderPass { INVALID_ID };
static PipelineHandle               InvalidPipeline { INVALID_ID };
// clang-format on

} // namespace Renderer
} // namespace Sogas