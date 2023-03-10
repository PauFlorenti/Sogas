#include "device_resource_pool.h"

#include "public/sgs_memory.h"

namespace Sogas
{
namespace Renderer
{
void ResourcePool::Init(Memory::Allocator* InAllocator, u32 InPoolSize, u32 InResourceSize)
{
    pool_size     = InPoolSize;
    resource_size = InResourceSize;
    allocator     = InAllocator;

    size_t size = pool_size * (resource_size * sizeof(u32));
    memory      = static_cast<u8*>(allocator->allocate(size, 1));
    memset(memory, 0, size);

    free_indices      = (u32*)(memory) + (pool_size * resource_size);
    free_indices_head = 0;

    for (u32 i = 0; i < pool_size; ++i)
    {
        free_indices[i] = i;
    }

    used_indices = 0;
}

void ResourcePool::Shutdown()
{
    if (free_indices_head != 0)
    {
        SWARNING("Resource pool has unfreed resources.");

        for (u32 i = 0; i < free_indices_head; ++i)
        {
            SWARNING("\tResource %u\n", free_indices[i]);
        }
    }

    SASSERT(used_indices == 0);

    allocator->deallocate(memory);
}

void ResourcePool::FreeAllResources() 
{
    free_indices_head = 0;
    used_indices = 0;

    for(u32 i = 0; i < pool_size; ++i)
    {
        free_indices[i] = i;
    }
}

u32 ResourcePool::ObtainResource()
{
    if (free_indices_head < pool_size)
    {
        const u32 free_index = free_indices[free_indices_head++];
        ++used_indices;
        return free_index;
    }

    SASSERT_MSG(false, "No more resources left.");
    return INVALID_ID;
}

void ResourcePool::ReleaseResource(u32 handle)
{
    free_indices[--free_indices_head] = handle;
    --used_indices;
}

void* ResourcePool::AccessResource(u32 handle)
{
    if (handle != INVALID_ID)
    {
        return &memory[handle * resource_size];
    }
    return nullptr;
}

const void* ResourcePool::AccessResource(u32 handle) const
{
    if (handle != INVALID_ID)
    {
        return &memory[handle * resource_size];
    }
    return nullptr;
}

} // namespace Renderer
} // namespace Sogas