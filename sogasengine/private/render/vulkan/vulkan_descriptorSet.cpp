#include "render/vulkan/vulkan_descriptorSet.h"
#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_pipeline.h"

namespace Sogas
{
namespace Vk
{

    void VulkanDescriptorSet::Create(const VulkanDevice* InDevice, DescriptorSet* InDescriptorSet, const Pipeline* InPipeline)
    {
        SASSERT(InDescriptorSet);
        SASSERT(InPipeline);

        auto internalState = std::make_shared<VulkanDescriptorSet>();
        InDescriptorSet->internalState = internalState;

        auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);

        internalState->pipelineLayout = pipelineInternalState.pipelineLayout;

        // Create descriptor pool
        VkDescriptorPoolSize poolSize;
        poolSize.descriptorCount    = 10;
        poolSize.type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.poolSizeCount  = 1;
        poolInfo.pPoolSizes     = &poolSize;
        poolInfo.maxSets        = 1;

        if (vkCreateDescriptorPool(InDevice->Handle, &poolInfo, nullptr, &internalState->descriptorPool) != VK_SUCCESS) {
            SERROR("Failed to create descriptor pool.");
            return;
        }

        VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool        = internalState->descriptorPool;
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = &pipelineInternalState.descriptorSetLayout;

        // Allocate descriptor set
        if (vkAllocateDescriptorSets(InDevice->Handle, &allocInfo, &internalState->descriptorSet) != VK_SUCCESS) {
            SERROR("Failed to allocate descriptor set.");
            return;
        }
    }

    void VulkanDescriptorSet::BindDescriptor(VkCommandBuffer cmd) const
    {
        vkCmdBindDescriptorSets(cmd, pipelineBindPoint, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }
} // Vk
} // Sogas
