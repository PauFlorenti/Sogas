#pragma once

namespace Sogas
{
namespace Renderer
{
class DeviceBuffer
{
  public:
    virtual void Release() = 0;
    virtual void SetData(void* data, const u64& size, const u64& offset = 0) = 0;
};
} // namespace Renderer
} // namespace Sogas