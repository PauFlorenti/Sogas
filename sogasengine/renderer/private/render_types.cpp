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

ShaderStateDescriptor& ShaderStateDescriptor::Reset()
{
    stages_count = 0;
    return *this;
}

ShaderStateDescriptor& ShaderStateDescriptor::SetName(const std::string& InName)
{
    name = InName;
    return *this;
}

ShaderStateDescriptor& ShaderStateDescriptor::AddStage(const char* InCode, u32 InSize, ShaderStageType InType)
{
    stages[stages_count].code = InCode;
    stages[stages_count].size = InSize;
    stages[stages_count].type = InType;
    ++stages_count;

    return *this;
}

ShaderStateDescriptor& ShaderStateDescriptor::SetSpvInput(bool InValue)
{
    spv_input = InValue;
    return *this;
}

} // namespace Renderer
} // namespace Sogas