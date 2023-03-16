
#include "renderpass.h"
#include "device_renderpass.h"

namespace Sogas
{
namespace Renderer
{

RenderPassDescriptor& RenderPassDescriptor::Reset()
{
    RenderTargetsCount = 0;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::AddRenderTexture(TextureHandle InHandle)
{
    OutputTextures[RenderTargetsCount] = InHandle;
    ++RenderTargetsCount;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::SetScaling(f32 InScaleX, f32 InScaleY, u8 InResize)
{
    ScaleX = InScaleX;
    ScaleY = InScaleY;
    Resize = InResize;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::SetDepthStencilTexture(TextureHandle InHandle)
{
    DepthStencilTexture = InHandle;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::SetName(std::string InName)
{
    Name = InName;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::SetType(RenderPassType InType)
{
    Type = InType;
    return *this;
}

RenderPassDescriptor& RenderPassDescriptor::SetOperations(RenderPassOperation InColor, RenderPassOperation InDepth, RenderPassOperation InStencil)
{
    ColorOperation   = InColor;
    DepthOperation   = InDepth;
    StencilOperation = InStencil;
    return *this;
}

RenderPass::RenderPass(RenderPassDescriptor InDescriptor)
: descriptor(InDescriptor)
{
}

RenderPass::~RenderPass()
{
    Destroy();
}

void RenderPass::Destroy()
{
    delete internalState;
    internalState = nullptr;
    descriptor    = {};
}
} // namespace Renderer
} // namespace Sogas