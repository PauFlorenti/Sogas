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

RenderPassOutput& RenderPassOutput::Reset()
{
    ColorFormatCounts = 0;
    return *this;
}

RenderPassOutput& RenderPassOutput::AddColor(Format InFormat)
{
    ColorFormats[ColorFormatCounts] = InFormat;
    ColorFormatCounts++;
    return *this;
}

RenderPassOutput& RenderPassOutput::SetDepth(Format InFormat)
{
    DepthStencilFormat = InFormat;
    return *this;
}

RenderPassOutput& RenderPassOutput::SetOperations(RenderPassOperation InColor, RenderPassOperation InDepth, RenderPassOperation InStencil)
{
    ColorOperation   = InColor;
    DepthOperation   = InDepth;
    StencilOperation = InStencil;
    return *this;
}

VertexInputDescriptor& VertexInputDescriptor::Reset()
{
    vertex_streams_count    = 0;
    vertex_attributes_count = 0;
    return *this;
}

VertexInputDescriptor& VertexInputDescriptor::AddVertexStream(const VertexStream& InVertexStream)
{
    vertex_stream[vertex_streams_count] = InVertexStream;
    ++vertex_streams_count;
    return *this;
}

VertexInputDescriptor& VertexInputDescriptor::AddVertexAttribute(const VertexAttribute& InVertexAttribute)
{
    vertex_attribute[vertex_attributes_count] = InVertexAttribute;
    ++vertex_attributes_count;
    return *this;
}

DepthStencilState& DepthStencilState::SetDepth(bool write, CompareOperation comparison_test)
{
    writeDepthEnabled = true;
    compareOp         = comparison_test;
    depthTestEnabled  = true;
    return *this;
}

} // namespace Renderer
} // namespace Sogas