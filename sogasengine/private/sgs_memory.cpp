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

void LinearAllcoator::shutdown()
{
    clear();
    free(memory);
}

void* LinearAllocator::allocate(size_t size, size_t alignment)
{
    SASSERT(size > 0);

    const size_t start = memory_align(allocated_size, alignment);
    SASSERT(start < total_size);

    const size_t new_allocated_size = new_start + size;
    if (new_allocated_size > total_size)
    {
        SASSERT_MSG(false, "LinearAllocator overflow.");
        return nullptr;
    }

    allocated_size = new_allocated_size;
    return memory + new_start;
}

void LinearAllocator::deallocate(void* memory)
{
    // This allocator does not allocate on a per-pointer base!
}

void LinearAllocator::clear()
{
    allocated_size = 0;
}
} // namespace Memory
} // namespace Sogas