#pragma once

namespace Sogas
{
namespace Renderer
{
class DeviceSwapchain
{
  public:
    virtual ~DeviceSwapchain() = default;
    virtual void Destroy() = 0;
};
} // namespace Renderer
} // namespace Sogas