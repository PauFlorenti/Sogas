#pragma once

#include "render_types.h"
#include "texture.h" // TODO At the moment used for Format enum. This should be moved, enums probably into another file.

namespace Sogas
{
namespace Renderer
{
class DeviceSwapchain;
class RenderPass;

struct SwapchainDescriptor
{
    i32    width;
    i32    height;
    u32    imageCount; // Number of buffers in the swapchain
    Format format;     // Swapchain images format
    f32    clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

class Swapchain final
{
  public:
    Swapchain() = default;
    Swapchain(SwapchainDescriptor descriptor);
    Swapchain(const Swapchain&)                  = delete;
    const Swapchain& operator=(const Swapchain&) = delete;
    Swapchain(Swapchain&& other);
    Swapchain& operator=(Swapchain&& other);
    ~Swapchain() { Destroy(); }

    void                       Destroy();
    const RenderPass*          GetRenderpass() const { return renderpass; }
    const SwapchainDescriptor& GetDescriptor() const { return descriptor; }
    void                       SetSwapchainSize(const u32 /*InWidth*/, const u32 /*InHeight*/){};

    DeviceSwapchain* internalState = nullptr;
    bool             resized{false};
    RenderPass*      renderpass = nullptr;

  private:
    SwapchainDescriptor descriptor;
};
} // namespace Renderer
} // namespace Sogas