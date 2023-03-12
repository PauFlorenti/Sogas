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

    u8*    memory         = nullptr;
    size_t allocated_size = 0;
    size_t total_size     = 0;
};

struct StackAllocator : public Allocator
{
    ~StackAllocator() override;

    void init(size_t size);
    void shutdown();

    void* allocate(size_t size, size_t alignment) override;
    void  deallocate(void* memory) override;

    size_t get_marker();
    void free_marker();
    void clear();

    u8*    memory         = nullptr;
    size_t allocated_size = 0;
    size_t total_size     = 0;
};

} // namespace Memory
} // namespace Sogas