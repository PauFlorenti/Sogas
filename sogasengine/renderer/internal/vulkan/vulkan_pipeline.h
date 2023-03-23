#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
class RenderPass;
namespace Vk
{
class VulkanDevice;
class VulkanDescriptorSetLayout;
class VulkanPipeline
{
  public:
    VulkanPipeline()                                             = default;
    VulkanPipeline(const VulkanPipeline&)                        = delete;
    VulkanPipeline(VulkanPipeline&&)                             = delete; // TODO move operations should be allowed.
    const VulkanPipeline& operator=(const VulkanPipeline& other) = delete;
    ~VulkanPipeline();

    static PipelineHandle Create(VulkanDevice* InDevice, const PipelineDescriptor& InDescriptor);

    void Destroy();

    // TODO rethink this, probably make them private ...
    VkPipeline          pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint bind_point;

    ShaderStateHandle shader_state;

    const VulkanDescriptorSetLayout* descriptor_set_layout[MAX_DESCRIPTOR_SET_LAYOUTS];
    DescriptorSetLayoutHandle        descriptor_set_layout_handle[MAX_DESCRIPTOR_SET_LAYOUTS];
    u32                              active_layout_count = 0;

    PipelineHandle handle;
    bool           graphics_pipeline = true;

  private:
    VulkanDevice* device = nullptr;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
