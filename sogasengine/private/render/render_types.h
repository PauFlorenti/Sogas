#pragma once

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
        R8_SNORM
    };

    enum class Usage
    {
        DEFAULT = 0,    // no CPU access, GPU read/write
        UPLOAD = 1,     // CPU write, GPU read
        READBACK = 2    // CPU read, GPU write
    };

    enum class BindPoint
    {
        NONE = 0,
        VERTEX = 1 << 0,
        INDEX = 1 << 1,
        UNIFORM = 1 << 2,
        RENDER_TARGET = 1 << 3,
        DEPTH_STENCIL = 1 << 4,
        SHADER_SAMPLE = 1 << 5
    };

    // Resource descriptors

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
        u32 width;
        u32 height;
        u32 imageCount; // Number of buffers in the swapchain
        Format format;  // Swapchain images format
        f32 clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
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
        GPUBufferDescriptor descriptor;
        std::weak_ptr<GPU_device> device;
    };

    struct Texture : public GPUResource
    {
        TextureDescriptor descriptor;
    };

    struct Attachment
    {
        const Texture* texture = nullptr;

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
            const Texture* texture,
            LoadOp loadop = LoadOp::LOAD,
            StoreOp storeop = StoreOp::STORE
        )
        {
            Attachment att;
            att.texture = texture;
            att.loadop = loadop;
            att.storeop = storeop;
            return att;
        }
    };

    // Objects

    struct Swapchain : public GPUBase
    {
        SwapchainDescriptor descriptor;
    };

    struct RenderPassDescriptor
    {
        std::vector<Attachment> attachments;
    };

    struct RenderPass : public GPUBase
    {
        RenderPassDescriptor descriptor;

    };

    struct Pipeline : public GPUBase
    {

    };

    struct Vertex
    {
        glm::vec3 position;
        //glm::vec3 normal;
        //glm::vec2 uvs;
        glm::vec4 color;

        bool operator==(const Vertex& other) const {
            return position == other.position && 
                //normal == other.normal &&
                //uvs == other.uvs &&
                color == other.color;
        }
    };

} // Sogas
