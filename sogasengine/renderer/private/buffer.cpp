#include "buffer.h"
#include "render_device.h"

namespace Sogas
{
namespace Renderer
{

BufferDescriptor& BufferDescriptor::reset()
{
    size = 0;
    data = nullptr;
    return *this;
}

BufferDescriptor& BufferDescriptor::set(BufferUsage InUsage, BufferBindingPoint InBindingPoint, u32 InSize)
{
    usage   = InUsage;
    binding = InBindingPoint;
    size    = InSize;
    return *this;
}

BufferDescriptor& BufferDescriptor::setData(void* InData)
{
    data = InData;
    return *this;
}

BufferDescriptor& BufferDescriptor::setName(const std::string& InName)
{
    name = InName;
    return *this;
}

} // namespace Renderer
} // namespace Sogas