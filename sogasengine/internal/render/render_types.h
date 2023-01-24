#pragma once

#include "resources/resource.h"

namespace Sogas 
{
    
    enum class GraphicsAPI
    {
        Vulkan = 0,
        OpenGL = 1,
        Dx11 = 2,
        Dx12 = 3
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
        SOLID = 0,
        SHADOW_CASTER = 1,
        TRANSPARENT = 2,
        COUNT = TRANSPARENT
    };

    enum class Format
    {
        UNDEFINED = 0,

        R32G32B32A32_SFLOAT,
        R32G32B32A32_SINT,
        R32G32B32A32_UINT,

        R32G32B32_SFLOAT,
        R32G32B32_SINT,
        R32G32B32_UINT,

        R32G32_SFLOAT,
        R32G32_SINT,
        R32G32_UINT,

        R32_SFLOAT,
        R32_SINT,
        R32_UINT,

        R16G16B16A16_SFLOAT,
        R16G16B16A16_SINT,
        R16G16B16A16_UINT,
        R16G16B16A16_SSCALED,
        R16G16B16A16_USCALED,
        R16G16B16A16_SNORM,
        R16G16B16A16_UNORM,

        R16G16B16_SFLOAT,
        R16G16B16_SINT,
        R16G16B16_UINT,
        R16G16B16_SSCALED,
        R16G16B16_USCALED,
        R16G16B16_SNORM,
        R16G16B16_UNORM,

        R16G16_SFLOAT,
        R16G16_SINT,
        R16G16_UINT,
        R16G16_SSCALED,
        R16G16_USCALED,
        R16G16_SNORM,
        R16G16_UNORM,

        R16_SFLOAT,
        R16_SINT,
        R16_UINT,
        R16_SSCALED,
        R16_USCALED,
        R16_SNORM,
        R16_UNORM,

        R8G8B8A8_SRGB,
        R8G8B8A8_SINT,
        R8G8B8A8_UINT,
        R8G8B8A8_SSCALED,
        R8G8B8A8_USCALED,
        R8G8B8A8_SNORM,
        R8G8B8A8_UNORM,

        R8G8B8_SRGB,
        R8G8B8_SINT,
        R8G8B8_UINT,
        R8G8B8_SSCALED,
        R8G8B8_USCALED,
        R8G8B8_UNORM,
        R8G8B8_SNORM,

        R8G8_SRGB,
        R8G8_SINT,
        R8G8_UINT,
        R8G8_SSCALED,
        R8G8_USCALED,
        R8G8_UNORM,
        R8G8_SNORM,

        R8_SRGB,
        R8_SINT,
        R8_UINT,
        R8_SSCALED,
        R8_USCALED,
        R8_UNORM,
        R8_SNORM,

        D32_SFLOAT,
        S8_UINT,
        D16_UNORM_S8_UINT,
        D24_UNORM_S8_UINT,
        D32_UNORM_S8_UINT
    };

    enum class Usage
    {
        DEFAULT = 0,    // no CPU access, GPU read/write
        UPLOAD = 1,     // CPU write, GPU read
        READBACK = 2    // CPU read, GPU write
    };

    enum class BindPoint
    {
        NONE            = 0,
        VERTEX          = 1 << 0,
        INDEX           = 1 << 1,
        UNIFORM         = 1 << 2,
        RENDER_TARGET   = 1 << 3,
        DEPTH_STENCIL   = 1 << 4,
        SHADER_SAMPLE   = 1 << 5
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

    struct Shader;
    struct Pipeline;

    struct GPUBufferDescriptor
    {
        u64 size;
        BindPoint bindPoint;
        Usage usage;
    };

    struct TextureDescriptor
    {
        enum TextureType {
            TEXTURE_TYPE_1D,
            TEXTURE_TYPE_2D,
            TEXTURE_TYPE_3D
        } textureType = TextureType::TEXTURE_TYPE_2D;

        u32 width = 0;
        u32 height = 0;
        u32 depth = 0;
        Format format = Format::UNDEFINED;
        Usage usage = Usage::DEFAULT;
        BindPoint bindPoint = BindPoint::NONE;
    };

    struct SwapchainDescriptor
    {
        i32 width;
        i32 height;
        u32 imageCount; // Number of buffers in the swapchain
        Format format;  // Swapchain images format
        f32 clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    };

    struct PushConstantDescriptor
    {
        u32 size{0};
        u32 offset{0};
        ShaderStage stage = ShaderStage::UNDEFINED;
    };

    struct GPUBuffer;
    struct Texture;
    struct DescriptorSet;
    struct Descriptor
    {
        u32                 binding     = 0;
        u32                 size        = 0;
        u32                 offset      = 0;
        u32                 count       = 0;
        UniformType         uniformType = UniformType::UNIFORM;
        ShaderStage         stage       = ShaderStage::UNDEFINED;
        const GPUBuffer*    buffer      = nullptr;
        const Texture*      texture     = nullptr;
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
        bool                depthTestEnabled        {false};
        bool                writeDepthEnabled       {false};
        bool                depthBoundTestEnabled   {false};
        bool                stencilTestEnabled      {false};
        f32                 minDepthBound           {0.0f};
        f32                 maxDepthBound           {1.0f};
        CompareOperations   compareOp               = CompareOperations::NEVER;
    };

    struct PipelineDescriptor
    {
        const Shader*               vs                  = nullptr;
        const Shader*               ps                  = nullptr;
        const RasterizationState*   rasterizationState  = nullptr;
        const DepthStencilState*    depthStencilState   = nullptr;
        std::string                 vertexDeclaration   = "";
    };

    // GPU Resources

    struct GPUBase
    {
        std::shared_ptr<void> internalState;
        bool IsValid() const { return internalState.get() != nullptr; }
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

    class GPU_device;

    struct GPUBuffer : public GPUResource
    {
        GPUBuffer()
        {
            resourceType    = ResourceType::BUFFER;
            mapdata         = nullptr;
        };

        GPUBufferDescriptor descriptor;
        std::weak_ptr<GPU_device> device;
    };

    struct Texture : public GPUResource, public IResource
    {
        Texture() {
            resourceType    = ResourceType::TEXTURE;
        };

        TextureDescriptor descriptor;
    };

    struct AttachmentFramebuffer;
    struct Attachment
    {
        enum class Type
        {
            RENDERTARGET,
            DEPTH_STENCIL
        } type = Type::RENDERTARGET;

        const Texture*  texture         = nullptr;
        const AttachmentFramebuffer* attachmentFramebuffer = nullptr;
        BindPoint       initialLayout   = BindPoint::NONE;
        BindPoint       subpassLayout   = BindPoint::NONE;
        BindPoint       finalLayout     = BindPoint::NONE;
        f32             clear[4]; // Clear attachment value.

        enum class LoadOp
        {
            LOAD,
            CLEAR,
            DONTCARE
        } loadop = LoadOp::LOAD;

        enum class StoreOp
        {
            STORE,
            DONTCARE
        } storeop = StoreOp::STORE;

        static Attachment RenderTarget(
            const Texture* texture = nullptr,
            const AttachmentFramebuffer* attachmentFramebuffer = nullptr,
            LoadOp loadop = LoadOp::CLEAR,
            StoreOp storeop = StoreOp::STORE,
            BindPoint initialLayout = BindPoint::NONE,
            BindPoint subpassLayout = BindPoint::RENDER_TARGET,
            BindPoint finalLayout = BindPoint::SHADER_SAMPLE
        )
        {
            Attachment att;
            att.type            = Attachment::Type::RENDERTARGET;
            att.texture         = texture;
            att.attachmentFramebuffer = attachmentFramebuffer,
            att.loadop          = loadop;
            att.storeop         = storeop;
            att.initialLayout   = initialLayout;
            att.subpassLayout   = subpassLayout;
            att.finalLayout     = finalLayout;
            return att;
        }

        static Attachment DepthStencil(
            const Texture* texture = nullptr,
            LoadOp loadop = LoadOp::DONTCARE,
            StoreOp storeop = StoreOp::DONTCARE,
            BindPoint initialLayout = BindPoint::DEPTH_STENCIL,
            BindPoint subpassLayout = BindPoint::DEPTH_STENCIL,
            BindPoint finalLayout = BindPoint::DEPTH_STENCIL
        )
        {
            Attachment att;
            att.type            = Attachment::Type::DEPTH_STENCIL;
            att.texture         = texture;
            att.loadop          = loadop;
            att.storeop         = storeop;
            att.initialLayout   = initialLayout;
            att.subpassLayout   = subpassLayout;
            att.finalLayout     = finalLayout;
            return att;
        }
    };

    // Objects

    struct RenderPassDescriptor
    {
        std::vector<Attachment> attachments;
    };

    struct AttachmentFramebuffer : public GPUBase
    {
        Format format;
        BindPoint usage = BindPoint::NONE;
    };

    struct RenderPass : public GPUBase
    {
        RenderPassDescriptor descriptor;
    };

    struct Swapchain : public GPUBase
    {
        SwapchainDescriptor descriptor;
        RenderPass renderpass;
        bool resized{false};

        void SetSwapchainSize(const i32 InWidth, const i32 InHeight) {
            descriptor.width    = InWidth;
            descriptor.height   = InHeight;
        }
    };

    struct Pipeline : public GPUBase
    {
        PipelineDescriptor descriptor;
    };

    struct Shader : public GPUBase
    {
        ShaderStage stage = ShaderStage::COUNT;
    };

    struct DescriptorSet : public GPUBase
    {
        bool dirty{false};
    };

    struct CommandBuffer
    {
        void * internalState;
        const Pipeline* activePipeline = nullptr;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uvs;
        glm::vec4 color;

        bool operator==(const Vertex& other) const {
            return position == other.position && 
                normal == other.normal &&
                uvs == other.uvs &&
                color == other.color;
        }
    };

} // Sogas
