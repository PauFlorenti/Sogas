#include "render_types.h"

namespace Sogas
{
namespace Renderer
{
BlendState&
BlendState::SetColor(BlendFactor InSourceColor, BlendFactor InDestinationColor, BlendOperation InColorOperation)
{
    return *this;
}
BlendState&
BlendState::SetAlpha(BlendFactor InSourceAlpha, BlendFactor InDestinationAlpha, BlendOperation InAlphaOperation)
{
    return *this;
}

BlendStateDescriptor& BlendStateDescriptor::Reset()
{
    ActiveStates = 0;
    return *this;
}

BlendState& BlendStateDescriptor::AddBlendState()
{
    return BlendStates[ActiveStates++];
}

} // namespace Renderer
} // namespace Sogas