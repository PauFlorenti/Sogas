#include "device_resource_pool.h"

namespace Sogas
{
namespace Renderer
{
void ResourcePool::Init(u32 InPoolSize, u32 InResourceSize) 
{
    pool_size = InPoolSize;
    resource_size = InResourceSize;

    size_t size = pool_size * (resource_size * sizeof(u32));
    memory = 
    memset(memory, 0, size);
}
} // namespace Renderer
} // namespace Sogas