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

PipelineDescriptor& PipelineDescriptor::AddDescriptorSetLayout(DescriptorSetLayoutHandle InDescriptorHandle)
{
    descriptor_set_layout[active_layouts_count] = InDescriptorHandle;
    ++active_layouts_count;
    return *this;
}

DepthStencilState& DepthStencilState::SetDepth(bool write, CompareOperation comparison_test)
{
    writeDepthEnabled = true;
    compareOp         = comparison_test;
    depthTestEnabled  = true;
    return *this;
}

SamplerDescriptor& SamplerDescriptor::SetMinMagMip(SamplerFilter InMin, SamplerFilter InMag, SamplerMipmapMode InMip)
{
    min_filter    = InMin;
    mag_filter    = InMag;
    mipmap_filter = InMip;
    return *this;
}

SamplerDescriptor& SamplerDescriptor::SetAddressModeU(SamplerAddressMode InU)
{
    address_mode_u = InU;
    return *this;
}

SamplerDescriptor& SamplerDescriptor::SetAddressModeUV(SamplerAddressMode InU, SamplerAddressMode InV)
{
    address_mode_u = InU;
    address_mode_v = InV;
    return *this;
}

SamplerDescriptor& SamplerDescriptor::SetAddressModeUVW(SamplerAddressMode InU, SamplerAddressMode InV, SamplerAddressMode InW)
{
    address_mode_u = InU;
    address_mode_v = InV;
    address_mode_w = InW;
    return *this;
}

SamplerDescriptor& SamplerDescriptor::SetName(const std::string& InName)
{
    name = InName;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::Reset()
{
    resources_count = 0;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::SetLayout(DescriptorSetLayoutHandle InLayout)
{
    layout = InLayout;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::Texture(TextureHandle InTexture, u16 InBinding)
{
    samplers[resources_count]  = INVALID_SAMPLER;
    bindings[resources_count]  = InBinding;
    resources[resources_count] = InTexture.index;
    ++resources_count;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::Buffer(BufferHandle InBuffer, u16 InBinding)
{
    bindings[resources_count] = InBinding;
    bindings[resources_count] = InBuffer.index;
    ++resources_count;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::TextureSampler(TextureHandle InTexture, SamplerHandle InSampler, u16 InBinding)
{
    samplers[resources_count]  = InSampler;
    bindings[resources_count]  = InBinding;
    resources[resources_count] = InTexture.index;
    ++resources_count;
    return *this;
    return *this;
}

DescriptorSetDescriptor& DescriptorSetDescriptor::SetName(std::string InName)
{
    name = InName;
    return *this;
}

DescriptorSetLayoutDescriptor& DescriptorSetLayoutDescriptor::Reset()
{
    bindings_count = 0;
    return *this;
}

DescriptorSetLayoutDescriptor& DescriptorSetLayoutDescriptor::AddBinding(const Binding& InBinding)
{
    bindings[bindings_count] = InBinding;
    ++bindings_count;
    return *this;
}

DescriptorSetLayoutDescriptor& DescriptorSetLayoutDescriptor::SetName(std::string InName)
{
    name = InName;
    return *this;
}

DescriptorSetLayoutDescriptor& DescriptorSetLayoutDescriptor::SetSetIndex(u32 InIndex)
{
    set_index = InIndex;
    return *this;
}

} // namespace Renderer
} // namespace Sogas