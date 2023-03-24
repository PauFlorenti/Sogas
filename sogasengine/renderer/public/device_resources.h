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
static BufferHandle                 INVALID_BUFFER { INVALID_ID };
static TextureHandle                INVALID_TEXTURE { INVALID_ID };
static ShaderStateHandle            INVALID_SHADERSTAGE { INVALID_ID };
static SamplerHandle                INVALID_SAMPLER { INVALID_ID };
static DescriptorSetHandle          INVALID_DESCRIPTORSET { INVALID_ID };
static DescriptorSetLayoutHandle    INVALID_DESCRIPTORSETLAYOUT { INVALID_ID };
static RenderPassHandle             INVALID_RENDERPASS { INVALID_ID };
static PipelineHandle               INVALID_PIPELINE { INVALID_ID };
// clang-format on

static const u8 MAX_SWAPCHAIN_IMAGES       = 3;
static const u8 MAX_SHADER_STAGES          = 5;
static const u8 MAX_IMAGE_OUTPUTS          = 8;
static const u8 MAX_DESCRIPTOR_SET_LAYOUTS = 8;
static const u8 MAX_DESCRIPTOR_PER_SET     = 16;
static const u8 MAX_VERTEX_STREAMS         = 16;
static const u8 MAX_VERTEX_ATTRIBUTE       = 16;

} // namespace Renderer
} // namespace Sogas