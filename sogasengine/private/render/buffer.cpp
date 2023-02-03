#include "render/buffer.h"

namespace Sogas
{
    namespace Renderer
    {
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

            internal_state.reset();
            internal_state = std::move(other.internal_state);
            other.internal_state.reset();
            sizeInBytes = other.sizeInBytes;
            device = std::move(other.device);
            other.device.reset();
            return *this;
        }

        size_t Buffer::getSizeInBytes() const
        {
            return static_cast<size_t>(sizeInBytes);
        }

        bool Buffer::isEmpty() const
        {
            return internal_state.get() == nullptr;
        }

        bool Buffer::isValid() const
        {
            return !isEmpty();
        }

        void Buffer::reset()
        {
            internal_state.reset();
        }

    } // namespace Renderer
} // namespace Sogas