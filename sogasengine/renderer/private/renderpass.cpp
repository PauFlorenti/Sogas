
#include "renderpass.h"
#include "device_renderpass.h"

namespace Sogas
{
namespace Renderer
{
RenderPass::RenderPass(RenderPassDescriptor InDescriptor) : descriptor(InDescriptor) {}

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