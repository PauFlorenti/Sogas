#pragma once
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;

struct DescriptorBinding
{
    VkDescriptorType type;
    u16              start = 0;
    u16              count = 0;
    u16              set   = 0;

    const char* name;
};

class VulkanDescriptorSetLayout
{
  public:
    VkDescriptorSetLayout         descriptor_set_layout;
    VkDescriptorSetLayoutBinding* binding        = nullptr;
    DescriptorBinding*            bindings       = nullptr;
    u16                           bindings_count = 0;
    u16                           set_index      = 0;

    DescriptorSetLayoutHandle handle;
};
class VulkanDescriptorSet
{
  public:
    explicit VulkanDescriptorSet(const u32 InSetNumber)
    : setNumber(InSetNumber){};
    VulkanDescriptorSet(const VulkanDescriptorSet&)                        = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&&)                             = delete;
    const VulkanDescriptorSet& operator=(const VulkanDescriptorSet& other) = delete;
    ~VulkanDescriptorSet();

    static DescriptorSetHandle       Create(VulkanDevice* InDevice, const DescriptorSetDescriptor& InDescriptor);
    static DescriptorSetLayoutHandle Create(VulkanDevice* InDevice, const DescriptorSetLayoutDescriptor& InDescriptor);

    void BindDescriptor(VkCommandBuffer cmd) const;

    const VkDescriptorSet& GetDescriptorSet() const
    {
        return descriptorSet;
    }

    ResourceHandle* resources = nullptr;
    SamplerHandle*  samplers  = nullptr;
    u16*            bindings  = nullptr;

    u32                              resources_count = 0;
    const VulkanDescriptorSetLayout* layout          = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkWriteDescriptorSet>         writes;
    const u32                                 setNumber;

  private:
    void FillWriteDescriptorSets(
      VulkanDevice*                    InDevice,
      const VulkanDescriptorSetLayout* InDescriptorSetLayout,
      VkDescriptorSet                  InDescriptorSet,
      VkWriteDescriptorSet*            InWriteDescriptorSet,
      VkDescriptorBufferInfo*          InBufferInfo,
      VkDescriptorImageInfo*           InImageInfo,
      u32&                             OutResourcesCount,
      const ResourceHandle*            InResources,
      const SamplerHandle*             InSamplers,
      const u16*                       InBindings);
      
    const VulkanDevice* device = nullptr;
    VkPipelineBindPoint pipelineBindPoint;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet     descriptorSet  = VK_NULL_HANDLE;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
