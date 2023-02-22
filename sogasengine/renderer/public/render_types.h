#pragma once

// TODO FIX THIS
#include "../internal/resources/resource.h"

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

enum class ShaderStage
{
    FRAGMENT = 0,
    VERTEX,
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
    u32         size{0};
    u32         offset{0};
    ShaderStage stage = ShaderStage::UNDEFINED;
};

struct DescriptorSet;
struct Descriptor
{
    u32         binding     = 0;
    u32         size        = 0;
    u32         offset      = 0;
    u32         count       = 0;
    UniformType uniformType = UniformType::UNIFORM;
    ShaderStage stage       = ShaderStage::UNDEFINED;
    // const Texture* texture     = nullptr;
};

enum CompareOperations
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

struct RasterizationState
{
    // TODO WIP
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

struct Shader;
struct PipelineDescriptor
{
    const Shader*             vs                 = nullptr;
    const Shader*             ps                 = nullptr;
    const RasterizationState* rasterizationState = nullptr;
    const DepthStencilState*  depthStencilState  = nullptr;
    std::string               vertexDeclaration  = "";
};

// GPU Resources

struct GPUBase
{
    std::shared_ptr<void> internalState;
    virtual void          Destroy() { internalState.reset(); };
    bool                  IsValid() const { return internalState.get() != nullptr; }
};

struct GPUResource : public GPUBase
{
    enum ResourceType
    {
        BUFFER,
        TEXTURE,
        UNKNOWN
    } resourceType = ResourceType::UNKNOWN;

    constexpr bool IsBuffer() const { return resourceType == ResourceType::BUFFER; }
    constexpr bool IsTexture() const { return resourceType == ResourceType::TEXTURE; }

    void* mapdata;
};

// Objects

struct Pipeline : public GPUBase
{
    PipelineDescriptor descriptor;
};

struct Shader : public GPUBase
{
    ~Shader() { Destroy(); }

    void Destroy() override { internalState.reset(); }

    ShaderStage stage = ShaderStage::COUNT;
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
