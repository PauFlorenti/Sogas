#pragma once
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{
class VulkanDevice;
class VulkanDescriptorSet
{
  public:
    explicit VulkanDescriptorSet(const u32 InSetNumber)
        : setNumber(InSetNumber){};
    VulkanDescriptorSet(const VulkanDescriptorSet&)                        = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&&)                             = delete;
    const VulkanDescriptorSet& operator=(const VulkanDescriptorSet& other) = delete;
    ~VulkanDescriptorSet();

    static DescriptorSetHandle Create(VulkanDevice* InDevice, const DescriptorSetDescriptor& InDescriptor);
    static DescriptorSetLayoutHandle Create(VulkanDevice* InDevice, const DescriptorSetLayoutDescriptor& InDescriptor);

    static void Create(const VulkanDevice*                              InDevice,
                       DescriptorSet*                                   InDescriptorSet,
                       VkDescriptorSetLayout                            InDescriptorSetLayout,
                       VkPipelineLayout                                 InPipelineLayout,
                       const std::vector<VkDescriptorSetLayoutBinding>& InBinding,
                       const u32                                        InSetNumber);

    static inline std::shared_ptr<VulkanDescriptorSet> ToInternal(const DescriptorSet* InDescriptorSet)
    {
        return std::static_pointer_cast<VulkanDescriptorSet>(InDescriptorSet->internalState);
    }

    void BindDescriptor(VkCommandBuffer cmd) const;

    const VkDescriptorSet& GetDescriptorSet() const { return descriptorSet; }

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkWriteDescriptorSet>         writes;
    const u32                                 setNumber;

  private:
    const VulkanDevice* device = nullptr;
    VkPipelineBindPoint pipelineBindPoint;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool    descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet     descriptorSet  = VK_NULL_HANDLE;
};
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
