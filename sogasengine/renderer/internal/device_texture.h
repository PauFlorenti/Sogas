#pragma once

namespace Sogas
{
namespace Renderer
{
class DeviceTexture
{
  public:
    virtual ~DeviceTexture() = default;
    virtual void Release()   = 0;
};
} // namespace Renderer
} // namespace Sogas