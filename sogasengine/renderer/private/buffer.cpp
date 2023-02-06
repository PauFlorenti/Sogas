#include "buffer.h"
#include "render_device.h"

namespace Sogas
{
    namespace Renderer
    {
        Buffer::Buffer(BufferDescriptor desc)
        : sizeInBytes(desc.elementSize * desc.size)
        {
        }

        Buffer::Buffer(Buffer &&other)
        {
            *this = std::move(other);
        }

        Buffer &Buffer::operator=(Buffer &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            device_buffer.reset();
            device_buffer = std::move(other.device_buffer);
            other.device_buffer.reset();
            sizeInBytes = other.sizeInBytes;
            device = std::move(other.device);
            other.device.reset();
            return *this;
        }

        Buffer::~Buffer()
        {
            sizeInBytes = 0;
        }

        size_t Buffer::getSizeInBytes() const
        {
            return static_cast<size_t>(sizeInBytes);
        }

        bool Buffer::isEmpty() const
        {
            return false;//device_buffer. == nullptr;
        }

        bool Buffer::isValid() const
        {
            return !isEmpty();
        }

        void Buffer::reset()
        {
            sizeInBytes = 0;
            device_buffer.reset();
        }

    } // namespace Renderer
} // namespace Sogas