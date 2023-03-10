#pragma once

namespace Sogas
{
namespace Memory
{

void   memory_copy(void* destination, void* source, size_t size);
size_t memory_align(size_t size, size_t alignment);

struct Allocator
{
    virtual ~Allocator(){};
    virtual void* allocate(size_t size, size_t alignment) = 0;
    virtual void  deallocate(void* memory)                = 0;
};

struct LinearAllocator : public Allocator
{
    ~LinearAllocator() override;

    void init(size_t size);
    void shutdown();

    void* allocate(size_t size, size_t alignment) override;
    void  deallocate(void* memory) override;

    void clear();

    void*  memory;
    size_t allocated_size = 0;
    size_t max_size       = 0;
};

} // namespace Memory
} // namespace Sogas