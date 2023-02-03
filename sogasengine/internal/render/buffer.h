#pragma once

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
            Buffer(const Buffer &) = delete;
            const Buffer &operator=(const Buffer &) = delete;
            Buffer(Buffer &&other);
            Buffer &operator=(Buffer &&other);

            size_t getSizeInBytes() const;
            bool isEmpty() const;
            bool isValid() const;
            void reset();

            std::shared_ptr<void> internal_state;
        private:
            u64 sizeInBytes{0};
            std::weak_ptr<GPU_device> device;
        };
    } // namespace Renderer
} // namespace Sogas
