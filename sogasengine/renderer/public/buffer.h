#pragma once

#include "device_buffer.h"

namespace Sogas
{
namespace Renderer
{
class GPU_device;

enum BufferBindingPoint
{
    Vertex  = 0,
    Index   = 1,
    Uniform = 2
};

enum BufferType
{
    Static  = 0,
    Dynamic = 1,
    Stream  = 2,
    Count
};

enum BufferUsage
{
    TRANSFER_DST,
    TRANSFER_SRC,
    INDEX,
    VERTEX,
    UNIFORM,
    UNDEFINED
};

struct BufferDescriptor
{
    BufferType         type{BufferType::Static};
    BufferUsage        usage{BufferUsage::UNDEFINED};
    BufferBindingPoint binding;

    u32 size{0};
    u32 elementSize{0};

    void*       data = nullptr;
    std::string name;

    BufferDescriptor& reset();
    BufferDescriptor& set(BufferUsage usage, BufferBindingPoint bindingPoint, u32 size);
    BufferDescriptor& setData(void* InData);
    BufferDescriptor& setName(const std::string& InName);
};

} // namespace Renderer
} // namespace Sogas
