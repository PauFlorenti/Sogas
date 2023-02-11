#pragma once

#include "device_buffer.h"

namespace Sogas
{
    namespace Renderer
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
            u64 size{0};
            u64 elementSize{0};
            BufferBindingPoint binding;
            BufferType type{BufferType::Static};
            BufferUsage usage{BufferUsage::UNDEFINED};
        };

        class Buffer final
        {
        public:
            Buffer() = default;
            Buffer(BufferDescriptor desc);
            Buffer(const Buffer &) = delete;
            const Buffer &operator=(const Buffer &) = delete;
            Buffer(Buffer &&other);
            Buffer &operator=(Buffer &&other);
            ~Buffer();

            void Release();

            size_t getSizeInBytes() const;
            bool isEmpty() const;
            bool isValid() const;

            // TODO This should not be public.
            std::shared_ptr<DeviceBuffer> device_buffer;
        private:
            u64 sizeInBytes{0};
            std::weak_ptr<GPU_device> device;
        };
    } // namespace Renderer
} // namespace Sogas