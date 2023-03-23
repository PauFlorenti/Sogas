#pragma once

#include "render_types.h" // TODO move attachment to own file.
//#include "attachment.h"

namespace Sogas
{
namespace Renderer
{
class DeviceRenderpass;

enum class RenderPassType
{
    GEOMETRY,
    COMPUTE,
    SWAPCHAIN
};

struct RenderPassDescriptor
{
    u32            RenderTargetsCount = 0;
    RenderPassType Type               = RenderPassType::GEOMETRY;

    TextureHandle OutputTextures[MAX_IMAGE_OUTPUTS];
    TextureHandle DepthStencilTexture;

    f32 ScaleX = 1.0f;
    f32 ScaleY = 1.0f;
    u8  Resize = 1;

    RenderPassOperation ColorOperation   = RenderPassOperation::DONTCARE;
    RenderPassOperation DepthOperation   = RenderPassOperation::DONTCARE;
    RenderPassOperation StencilOperation = RenderPassOperation::DONTCARE;

    std::string Name;

    RenderPassDescriptor& Reset();
    RenderPassDescriptor& AddRenderTexture(TextureHandle InHandle);
    RenderPassDescriptor& SetScaling(f32 InScaleX, f32 InScaleY, u8 InResize);
    RenderPassDescriptor& SetDepthStencilTexture(TextureHandle InHandle);
    RenderPassDescriptor& SetName(std::string InName);
    RenderPassDescriptor& SetType(RenderPassType InType);
    RenderPassDescriptor& SetOperations(RenderPassOperation InColor, RenderPassOperation InDepth, RenderPassOperation InStencil);

    // std::vector<Attachment>
    //   attachments;
};
} // namespace Renderer
} // namespace Sogas