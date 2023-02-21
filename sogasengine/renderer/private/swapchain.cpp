
#include "swapchain.h"
#include "device_swapchain.h"
#include "renderpass.h"

namespace Sogas
{
namespace Renderer
{
Swapchain::Swapchain(SwapchainDescriptor InDescriptor) : descriptor(InDescriptor), renderpass(new RenderPass()) {}

Swapchain::Swapchain(Swapchain&& other)
{
    *this = std::move(other);
}

Swapchain& Swapchain::operator=(Swapchain&& other)
{

    if (this == &other)
    {
        return *this;
    }

    internalState = std::move(other.internalState);
    renderpass    = std::move(other.renderpass);
    descriptor    = std::move(other.descriptor);

    other.internalState = nullptr;
    other.renderpass    = nullptr;
    other.descriptor    = {};

    return *this;
}

void Swapchain::Destroy()
{
    delete internalState;
    delete renderpass;
    internalState = nullptr;
    renderpass    = nullptr;
    descriptor    = {};
}
} // namespace Renderer
} // namespace Sogas