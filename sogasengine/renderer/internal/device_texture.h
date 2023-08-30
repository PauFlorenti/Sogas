#pragma once

namespace Sogas
{
namespace Renderer
{
struct TextureDescriptor;
class DeviceTexture
{
  public:
    virtual ~DeviceTexture()         = default;
    virtual void Release()           = 0;
    virtual void SetData(void* data) = 0;
};
} // namespace Renderer
} // namespace Sogas