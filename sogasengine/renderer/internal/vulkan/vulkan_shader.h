#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{

namespace Vk
{
class VulkanDevice;

struct VulkanShaderState
{
    VkPipelineShaderStageCreateInfo ShaderStageInfo[MAX_SHADER_STAGES];

    std::string Name;
    u8          ActiveShaders{0};
    bool        bIsGraphicsPipeline{false};
};

class VulkanShader
{
  public:
    VulkanShader()                                           = default;

    static std::vector<i8> ReadBinaryShaderFile(const std::string& InFilename);
    static char* ReadShaderFile(const std::string& InFilename, u32& OutSize);
    static ShaderStateHandle Create(VulkanDevice* InDevice, const ShaderStateDescriptor& InDescriptor);
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
