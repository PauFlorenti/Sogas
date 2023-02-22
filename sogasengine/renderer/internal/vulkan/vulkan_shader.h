#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{

namespace Vk
{
class VulkanDevice;

class VulkanShader
{
  public:
    VulkanShader()                                           = default;
    VulkanShader(const VulkanShader&)                        = delete;
    VulkanShader(VulkanShader&&)                             = delete;
    const VulkanShader& operator=(const VulkanShader& other) = delete;
    ~VulkanShader();

    static void Create(const VulkanDevice* device, ShaderStage stage, std::string InFilename, Shader* OutShader);

    static inline std::shared_ptr<VulkanShader> ToInternal(const Shader* InShader)
    {
        return std::static_pointer_cast<VulkanShader>(InShader->internalState);
    }

    VkShaderModule shaderModule;

    // Sets limited to 8 by now.
    std::vector<VkDescriptorSetLayoutBinding> layoutBindingsPerSet[8];
    std::vector<VkPushConstantRange>          pushConstantRanges;

  private:
    const VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
