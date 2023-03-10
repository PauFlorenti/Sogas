#pragma once

namespace Sogas
{
namespace Renderer
{

typedef u32 ResourceHandle;

// Resource Handles
struct BufferHandle
{
    ResourceHandle index;
};

struct TextureHandle
{
    ResourceHandle index;
};

// Invalid Handles

static BufferHandle InvalidBuffer{ INVALID_ID };
static TextureHandle InvalidTexture{ INVALID_ID };

} // namespace Renderer
} // namespace Sogas