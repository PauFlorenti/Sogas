#pragma once

namespace Sogas
{
namespace Renderer
{
class DeviceRenderpass
{
  public:
    virtual ~DeviceRenderpass() = default;
    virtual void Destroy() = 0;
};
} // namespace Renderer
} // namespace Sogas