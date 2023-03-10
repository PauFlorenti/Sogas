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

class Buffer final
{
  public:
    Buffer() = default;
    Buffer(BufferDescriptor desc);
    Buffer(Buffer&& other);
    Buffer& operator=(Buffer&& other);
    ~Buffer();

    void Release();
    void SetData(void* InData, const u64& size, const u64& offset = 0);

    const u32 Size() const;
    const u64 ByteSize() const;
    const u64 ElementSize() const;
    bool      isEmpty() const;
    bool      isValid() const;

    // TODO This should not be public.
    std::shared_ptr<DeviceBuffer> device_buffer;

  private:
    u64                       elementSize{0};
    u32                       elementCount{0};
    std::weak_ptr<GPU_device> device;
};
} // namespace Renderer
} // namespace Sogas
