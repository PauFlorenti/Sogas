#pragma once

namespace Sogas
{

    class GPU_device;

    enum BufferBindingPoint
    {
        Vertex = 0,
        Index = 1,
        Uniform = 2
    };

    enum BufferType
    {
        Static = 0,
        Dynamic = 1
    };

    class Buffer final
    {
        u32 id;
        size_t sizeInBytes{0};
        std::weak_ptr<GPU_device> device;

    public:
        Buffer();
        Buffer(const u32 id, const size_t size);
        ~Buffer();

        u32 getId() const { return id; }
        size_t getSizeInBytes() const { return sizeInBytes; }
    };
} // Sogas
