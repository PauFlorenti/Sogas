#include "renderpass.h"

namespace Sogas
{
namespace Renderer
{
RenderPass::RenderPass(RenderPassDescriptor InDescriptor) : descriptor(InDescriptor) {}

void RenderPass::Destroy()
{
    delete internalState;
    internalState = nullptr;
    descriptor    = {};
}
} // namespace Renderer
} // namespace Sogas