#pragma once

#include "sgs_memory.h"

namespace Sogas
{
namespace Renderer
{
struct ResourcePool
{
    void Init(u32 InPoolSize, u32 InResourceSize);
    void Shutdown();

    u32  ObtainResource();
    void ReleaseResource();
    void FreeAllResources();

    void*       AccessResource();
    const void* AccessResource() const;

    u8*        memory       = nullptr;
    u32*       free_indices = nullptr;
    Allocator* allocator    = nullptr;

    u32 free_indices_head = 0;
    u32 pool_size         = 16;
    u32 resource_size     = 4;
    u32 used_indices      = 0;
};
} // namespace Renderer
} // namespace Sogas