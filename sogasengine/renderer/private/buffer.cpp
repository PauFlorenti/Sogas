#include "buffer.h"
#include "render_device.h"

namespace Sogas
{
namespace Renderer
{
Buffer::Buffer(BufferDescriptor desc)
    : elementSize(desc.elementSize),
      elementCount(desc.size)
{}

Buffer::Buffer(Buffer&& other)
{
    *this = std::move(other);
}

Buffer& Buffer::operator=(Buffer&& other)
{
    if (this == &other)
    {
        return *this;
    }

    device_buffer.reset();
    device_buffer = std::move(other.device_buffer);
    other.device_buffer.reset();
    device = std::move(other.device);
    other.device.reset();

    elementSize        = std::move(other.elementSize);
    elementCount       = std::move(other.elementCount);
    other.elementCount = 0;
    other.elementSize  = 0;

    return *this;
}

Buffer::~Buffer()
{
    Release();
}

const u32 Buffer::Size() const
{
    return elementCount;
}

const u64 Buffer::ByteSize() const
{
    return elementSize * elementCount;
}

const u64 Buffer::ElementSize() const
{
    return elementSize;
}

bool Buffer::isEmpty() const
{
    return device_buffer == false;
}

bool Buffer::isValid() const
{
    return !isEmpty();
}

void Buffer::Release()
{
    device.reset();
    device_buffer.reset();
    elementCount = 0;
    elementSize  = 0;
}

} // namespace Renderer
} // namespace Sogas