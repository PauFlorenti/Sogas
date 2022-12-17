#pragma once

namespace Sogas 
{
    enum GraphicsAPI
    {
        Vulkan = 0,
        OpenGL = 1,
        Dx11 = 2,
        Dx12 = 3
    };

    enum PrimitiveTopology
    {
        UNDEFINED,
        TRIANGLELIST,
        TRIANGLESTRIP,
        TRIANGLEFAN,
        POINTLIST,
        LINELIST,
        LINESTRIP
    };

    enum DrawChannel
    {
        SOLID = 0,
        SHADOW_CASTER = 1,
        TRANSPARENT = 2,
        COUNT = TRANSPARENT
    };

    enum Usage
    {
        DEFAULT = 0,    // no CPU access, GPU read/write
        UPLOAD = 1,     // CPU write, GPU read
        READBACK = 2    // CPU read, GPU write
    };

    enum BindPoint
    {
        VERTEX,
        INDEX,
        UNIFORM
    };

    // Resource descriptors

    struct GPUBufferDescriptor
    {
        u64 size;
        BindPoint bindPoint;
        Usage usage;
    };

    // GPU Resources

    struct GPUResource
    {
        enum Type
        {
            BUFFER,
            TEXTURE,
            UNKNOWN
        } type = Type::UNKNOWN;

        std::shared_ptr<void> internalState;
        bool IsValid() const { return internalState.get() != nullptr; }
        constexpr bool IsBuffer() const { return type == Type::BUFFER; }
        constexpr bool IsTexture() const { return type == Type::TEXTURE; }

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
