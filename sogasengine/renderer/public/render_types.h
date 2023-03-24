#pragma once

// TODO FIX THIS
#include "../internal/resources/resource.h"

#include "device_resources.h"

namespace Sogas
{
namespace Renderer
{

enum class GraphicsAPI
{
    Vulkan = 0,
    OpenGL = 1,
    Dx11   = 2,
    Dx12   = 3
};

enum class PrimitiveTopology
{
    UNDEFINED,
    TRIANGLELIST,
    TRIANGLESTRIP,
    TRIANGLEFAN,
    POINTLIST,
    LINELIST,
    LINESTRIP
};

enum class DrawChannel
{
    SOLID         = 0,
    SHADOW_CASTER = 1,
    TRANSPARENT   = 2,
    COUNT         = TRANSPARENT
};

enum class ShaderStageType
{
    FRAGMENT = 0,
    VERTEX,
    COMPUTE,
    UNDEFINED,
    COUNT = UNDEFINED
};

enum class UniformType
{
    UNIFORM = 0,
    SAMPLED
};

enum class RenderPassOperation
{
    DONTCARE,
    LOAD,
    CLEAR,
    COUNT
};

enum class VertexInputRate
{
    PER_VERTEX,
    PER_INSTANCE,
    COUNT
};

enum class VertexFormat
{
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    MAT4,
    BYTE,
    BYTE4N,
    UBYTE,
    UBYTE4N,
    SHORT2,
    SHORT2N,
    SHORT4,
    SHORT4N,
    UINT,
    UINT2,
    UINT4,
    COUNT
};

// Resource descriptors

struct PushConstantDescriptor
{
    u32             size{0};
    u32             offset{0};
    ShaderStageType stage = ShaderStageType::UNDEFINED;
};

struct DescriptorSet;
struct Descriptor
{
    u32             binding     = 0;
    u32             size        = 0;
    u32             offset      = 0;
    u32             count       = 0;
    UniformType     uniformType = UniformType::UNIFORM;
    ShaderStageType stage       = ShaderStageType::UNDEFINED;
    // const Texture* texture     = nullptr;
};

struct ShaderStage
{
    const char*     code = nullptr;
    u32             size = 0;
    ShaderStageType type = ShaderStageType::UNDEFINED;
};

struct ShaderStateDescriptor
{
    ShaderStage stages[MAX_SHADER_STAGES];
    std::string name;
    u32         stages_count = 0;
    u32         spv_input    = 0;

    ShaderStateDescriptor& Reset();
    ShaderStateDescriptor& SetName(const std::string& InName);
    ShaderStateDescriptor& AddStage(const char* InCode, u32 InSize, ShaderStageType InType);
    ShaderStateDescriptor& SetSpvInput(bool InValue);
};

struct SamplerDescriptor
{
    enum class SamplerFilter
    {
        NEAREST,
        LINEAR,
        CUBIC,
        COUNT
    };

    enum class SamplerMipmapMode
    {
        NEAREST,
        LINEAR,
        COUNT
    };

    enum class SamplerAddressMode
    {
        REPEAT = 0,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE,
        COUNT
    };

    SamplerFilter     min_filter    = SamplerFilter::NEAREST;
    SamplerFilter     mag_filter    = SamplerFilter::NEAREST;
    SamplerMipmapMode mipmap_filter = SamplerMipmapMode::NEAREST;

    SamplerAddressMode address_mode_u = SamplerAddressMode::REPEAT;
    SamplerAddressMode address_mode_v = SamplerAddressMode::REPEAT;
    SamplerAddressMode address_mode_w = SamplerAddressMode::REPEAT;

    std::string name;

    SamplerDescriptor& SetMinMagMip(SamplerFilter InMin, SamplerFilter InMag, SamplerMipmapMode InMip);
    SamplerDescriptor& SetAddressModeU(SamplerAddressMode InU);
    SamplerDescriptor& SetAddressModeUV(SamplerAddressMode InU, SamplerAddressMode InV);
    SamplerDescriptor& SetAddressModeUVW(SamplerAddressMode InU, SamplerAddressMode InV, SamplerAddressMode InW);
    SamplerDescriptor& SetName(const std::string& InName);
};

enum class DescriptorType
{
    SAMPLER = 0,
    COMBINED_IMAGE_SAMPLER,
    SAMPLED_IMAGE,
    STORAGE_IMAGE,
    UNIFORM_TEXEL_BUFFER,
    STORAGE_TEXEL_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    UNIFOR_BUFFER_DYNAMIC,
    STORAGE_BUFFER_DYNAMIC,
    ATTACHMENT,
    COUNT
};

struct DescriptorSetDescriptor
{
    ResourceHandle resources[MAX_DESCRIPTOR_PER_SET];
    SamplerHandle  samplers[MAX_DESCRIPTOR_PER_SET];
    u16            bindings[MAX_DESCRIPTOR_PER_SET];

    DescriptorSetLayoutHandle layout;
    u32 resources_count = 0;

    std::string name;

    DescriptorSetDescriptor& Reset();
    DescriptorSetDescriptor& SetLayout(DescriptorSetLayoutHandle InLayout);
    DescriptorSetDescriptor& Texture(TextureHandle InTexture, u16 InBinding);
    DescriptorSetDescriptor& Buffer(BufferHandle InBuffer, u16 InBinding);
    DescriptorSetDescriptor& TextureSampler(TextureHandle InTexture, SamplerHandle InSampler, u16 InBinding);
    DescriptorSetDescriptor& SetName(std::string InName);

};
struct DescriptorSetLayoutDescriptor
{
    struct Binding
    {
        DescriptorType type  = DescriptorType::COUNT;
        u16            start = 0;
        u16            count = 0;
        std::string    name;
    };

    Binding bindings[MAX_DESCRIPTOR_PER_SET];
    u32     bindings_count = 0;
    u32     set_index      = 0;

    std::string name;

    DescriptorSetLayoutDescriptor& Reset();
    DescriptorSetLayoutDescriptor& AddBinding(const Binding& InBinding);
    DescriptorSetLayoutDescriptor& SetName(std::string InName);
    DescriptorSetLayoutDescriptor& SetSetIndex(u32 InIndex);
};

enum class CompareOperation
{
    NEVER = 0,
    LESS,
    EQUAL,
    LESS_OR_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_OR_EQUAL,
    ALWAYS
};

enum class StencilOperation
{
    KEEP = 0,
    ZERO,
    REPLACE,
    INCREMENT_AND_CLAMP,
    DECREMENT_AND_CLAMP,
    INVERT,
    INCREMENT_AND_WRAP,
    DECREMENT_AND_WRAP,
    COUNT
};

enum class CullMode
{
    NONE           = 0x00000000,
    FRONT          = 0x00000001,
    BACK           = 0x00000002,
    FRONT_AND_BACK = 0x00000003,
    COUNT
};

enum class FrontFace
{
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE         = 1,
    COUNT
};

enum class FillMode
{
    WIREFRAME,
    SOLID,
    POINT,
    COUNT
};

struct RasterizationState
{
    CullMode  cull_mode  = CullMode::NONE;
    FrontFace front_face = FrontFace::COUNTER_CLOCKWISE;
    FillMode  fill_mode  = FillMode::SOLID;
};

struct StencilOperationState
{
    StencilOperation fail       = StencilOperation::KEEP;
    StencilOperation pass       = StencilOperation::KEEP;
    StencilOperation depth_fail = StencilOperation::KEEP;
    CompareOperation compare    = CompareOperation::ALWAYS;

    u32 compare_mask = 0xff;
    u32 write_mask   = 0xff;
    u32 reference    = 0xff;
};

struct DepthStencilState
{
    bool depthTestEnabled{false};
    bool writeDepthEnabled{false};
    bool depthBoundTestEnabled{false};
    bool stencilTestEnabled{false};
    f32  minDepthBound{0.0f};
    f32  maxDepthBound{1.0f};

    CompareOperation      compareOp = CompareOperation::NEVER;
    StencilOperationState front;
    StencilOperationState back;

    DepthStencilState()
    : depthTestEnabled(false),
      writeDepthEnabled(false),
      stencilTestEnabled(false)
    {
    }

    DepthStencilState& SetDepth(bool write, CompareOperation comparisont_test);
};

enum class BlendFactor
{
    ONE,
    COUNT
};

enum class BlendOperation
{
    ADD = 0,
    SUBSTRACT,
    REVERSE_SUBSTRACT,
    MIN,
    MAX,
    COUNT
};

struct BlendState
{
    BlendFactor    SourceColor      = BlendFactor::ONE;
    BlendFactor    DestinationColor = BlendFactor::ONE;
    BlendOperation ColorOperation   = BlendOperation::ADD;

    BlendFactor    SourceAlpha      = BlendFactor::ONE;
    BlendFactor    DestinationAlpha = BlendFactor::ONE;
    BlendOperation AlphaOperation   = BlendOperation::ADD;

    bool BlendEnabled  = true;
    bool SeparateBlend = true;
    u8   pad           = 6;

    // TODO mask ...

    BlendState()
    : BlendEnabled(false){};

    BlendState& SetColor(BlendFactor InSourceColor, BlendFactor InDestinationColor, BlendOperation InColorOperation);
    BlendState& SetAlpha(BlendFactor InSourceAlpha, BlendFactor InDestinationAlpha, BlendOperation InAlphaOperation);
    // TODO Set mask ...
};

struct BlendStateDescriptor
{
    // TODO make it into constant values file ...
    BlendState BlendStates[8];
    u32        ActiveStates = 0;

    BlendStateDescriptor& Reset();
    BlendState&           AddBlendState();
};

enum class Format;
struct RenderPassOutput
{
    Format ColorFormats[MAX_IMAGE_OUTPUTS];
    Format DepthStencilFormat;
    u32    ColorFormatCounts;

    RenderPassOperation ColorOperation   = RenderPassOperation::DONTCARE;
    RenderPassOperation DepthOperation   = RenderPassOperation::DONTCARE;
    RenderPassOperation StencilOperation = RenderPassOperation::DONTCARE;

    RenderPassOutput& Reset();
    RenderPassOutput& AddColor(Format InFormat);
    RenderPassOutput& SetDepth(Format InFormat);
    RenderPassOutput& SetOperations(RenderPassOperation InColor, RenderPassOperation InDepth, RenderPassOperation InStencil);
};

struct VertexStream
{
    u16             binding    = 0;
    u16             stride     = 0;
    VertexInputRate input_rate = VertexInputRate::COUNT;
};

struct VertexAttribute
{
    u16          location = 0;
    u16          binding  = 0;
    u32          offset   = 0;
    VertexFormat format   = VertexFormat::COUNT;
};

struct VertexInputDescriptor
{
    u32 vertex_streams_count    = 0;
    u32 vertex_attributes_count = 0;

    VertexStream    vertex_stream[MAX_VERTEX_STREAMS];
    VertexAttribute vertex_attribute[MAX_VERTEX_ATTRIBUTE];

    VertexInputDescriptor& Reset();
    VertexInputDescriptor& AddVertexStream(const VertexStream& InVertexStream);
    VertexInputDescriptor& AddVertexAttribute(const VertexAttribute& InVertexAttribute);
};

struct PipelineDescriptor
{
    RasterizationState    rasterizationState;
    DepthStencilState     depthStencilState;
    VertexInputDescriptor vertexInputState;
    BlendStateDescriptor  blendState;
    ShaderStateDescriptor shaders;

    RenderPassOutput          render_pass;
    DescriptorSetLayoutHandle descriptor_set_layout[MAX_DESCRIPTOR_SET_LAYOUTS];

    std::string name;

    u32 active_layouts_count = 0;

    PipelineDescriptor& AddDescriptorSetLayout(DescriptorSetLayoutHandle InHandle);
};

// GPU Resources

struct GPUBase
{
    std::shared_ptr<void> internalState;
    virtual void          Destroy()
    {
        internalState.reset();
    };
    bool IsValid() const
    {
        return internalState.get() != nullptr;
    }
};

struct GPUResource : public GPUBase
{
    enum ResourceType
    {
        BUFFER,
        TEXTURE,
        UNKNOWN
    } resourceType = ResourceType::UNKNOWN;

    constexpr bool IsBuffer() const
    {
        return resourceType == ResourceType::BUFFER;
    }
    constexpr bool IsTexture() const
    {
        return resourceType == ResourceType::TEXTURE;
    }

    void* mapdata;
};

struct VertexLayout
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uvs;
    glm::vec4 color;

    bool operator==(const VertexLayout& other) const
    {
        return position == other.position && normal == other.normal && uvs == other.uvs && color == other.color;
    }
};
} // namespace Renderer
} // namespace Sogas
