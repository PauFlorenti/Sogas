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
};
struct DescriptorSetDescriptor
{
};
struct DescriptorSetLayoutDescriptor
{
};

enum class CompareOperations
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

struct DepthStencilState
{
    bool              depthTestEnabled{false};
    bool              writeDepthEnabled{false};
    bool              depthBoundTestEnabled{false};
    bool              stencilTestEnabled{false};
    f32               minDepthBound{0.0f};
    f32               maxDepthBound{1.0f};
    CompareOperations compareOp = CompareOperations::NEVER;
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

    bool BlendEnabled = true;

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

struct Shader;
struct PipelineDescriptor
{
    const Shader*             vs                 = nullptr;
    const Shader*             ps                 = nullptr;
    const RasterizationState* rasterizationState = nullptr;
    const DepthStencilState*  depthStencilState  = nullptr;
    const BlendState*         blendState         = nullptr;
    std::string               vertexDeclaration  = "";
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

// Objects

struct Pipeline : public GPUBase
{
    PipelineDescriptor descriptor;
};

struct Shader : public GPUBase
{
    ~Shader()
    {
        Destroy();
    }

    void Destroy() override
    {
        internalState.reset();
    }

    ShaderStageType stage = ShaderStageType::COUNT;
};

struct DescriptorSet : public GPUBase
{
    bool dirty{false};
};

struct CommandBuffer
{
    void*           internalState;
    const Pipeline* activePipeline = nullptr;
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
