#include "sgs_memory.h"

namespace Sogas
{
namespace Memory
{
void memory_copy(void* destination, void* source, size_t size)
{
    memcpy(destination, source, size);
}

size_t memory_align(size_t size, size_t alignment)
{
    const size_t mask = alignment - 1;
    return (size + alignment) & ~mask;
}

LinearAllocator::~LinearAllocator() {}

void LinearAllocator::init(size_t size)
{
    memory         = static_cast<u8*>(malloc(size));
    total_size     = size;
    allocated_size = 0;
}

void LinearAllocator::shutdown()
{
    clear();
    free(memory);
}

void* LinearAllocator::allocate(size_t size, size_t alignment)
{
    SASSERT(size > 0);

    const size_t new_start = memory_align(allocated_size, alignment);
    SASSERT(new_start < total_size);

    const size_t new_allocated_size = new_start + size;
    if (new_allocated_size > total_size)
    {
        SASSERT_MSG(false, "LinearAllocator overflow.");
        return nullptr;
    }

    allocated_size = new_allocated_size;
    return memory + new_start;
}

void LinearAllocator::deallocate(void* /*memory*/)
{
    // This allocator does not allocate on a per-pointer base!
}

void LinearAllocator::clear()
{
    allocated_size = 0;
}

StackAllocator::~StackAllocator() {}

void StackAllocator::init(size_t size)
{
    memory         = static_cast<u8*>(malloc(size));
    total_size     = size;
    allocated_size = 0;
}

void StackAllocator::shutdown()
{

}

void* StackAllocator::allocate(size_t size, size_t alignment)
{
    SASSERT(size > 0);
    
    auto new_start = memory_align(allocated_size, alignment);
    SASSERT(new_start < total_size);

    const auto new_allocated_size = new_start + size;
    SASSERT_MSG(new_allocated_size < total_size, "StackAllocator Overflow!");

    allocated_size = new_allocated_size;
    return memory + new_start;
}

void StackAllocator::deallocate(void* pointer)
{
    SASSERT(memory <= pointer);
    SASSERT_MSG(pointer < memory + total_size, "Out of bound total size.");
    SASSERT_MSG(pointer < memory + allocated_size, "Out of bound allocated size.");

    const size_t size_at_pointer = (u8*)pointer - memory;
    allocated_size = size_at_pointer;
}

size_t StackAllocator::get_marker()
{
    return allocated_size;
}

void StackAllocator::free_marker()
{

}

void StackAllocator::clear()
{
    allocated_size = 0;
}

} // namespace Memory
} // namespace Sogas