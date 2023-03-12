#pragma once

namespace Sogas
{
namespace Memory
{
struct Allocator;
}
namespace Renderer
{
struct ResourcePool
{
    void Init(Memory::Allocator* InAllocator, u32 InPoolSize, u32 InResourceSize);
    void Shutdown();

    u32  ObtainResource();
    void ReleaseResource(u32 handle);
    void FreeAllResources();

    void*       AccessResource(u32 handle);
    const void* AccessResource(u32 handle) const;

    u8*        memory       = nullptr;
    u32*       free_indices = nullptr;
    Memory::Allocator* allocator    = nullptr;

    u32 free_indices_head = 0;
    u32 pool_size         = 16;
    u32 resource_size     = 4;
    u32 used_indices      = 0;
};
} // namespace Renderer
} // namespace Sogas