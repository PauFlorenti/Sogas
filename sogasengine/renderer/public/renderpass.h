#pragma once

#include "render_types.h" // TODO move attachment to own file.

namespace Sogas
{
namespace Renderer
{
class DeviceRenderpass;
struct RenderPassDescriptor
{
    std::vector<Sogas::Attachment> attachments;
};
class RenderPass final
{
  public:
    RenderPass() = default;
    RenderPass(RenderPassDescriptor InDescriptor);
    ~RenderPass();

    void Destroy();

    const RenderPassDescriptor& GetDescriptor() const { return descriptor; }
    void AddAttachment(Sogas::Attachment InAttachment) { descriptor.attachments.emplace_back(InAttachment); }
    DeviceRenderpass* internalState = nullptr;

  private:
    RenderPassDescriptor descriptor;
};
} // namespace Renderer
} // namespace Sogas