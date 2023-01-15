#include "render/vulkan/vulkan_descriptorSet.h"
#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_pipeline.h"

namespace Sogas
{
namespace Vk
{

    void VulkanDescriptorSet::Create(
        const VulkanDevice* InDevice, 
        DescriptorSet* InDescriptorSet, 
        VkDescriptorSetLayout InDescriptorSetLayout, 
        VkPipelineLayout InPipelineLayout, 
        const std::vector<VkDescriptorSetLayoutBinding>& InBindings,
        const u32 InSetNumber)
    {
        SASSERT(InDescriptorSet);

        auto internalState = std::make_shared<VulkanDescriptorSet>(InSetNumber);
        InDescriptorSet->internalState = internalState;
        internalState->pipelineLayout = InPipelineLayout;
        internalState->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        u32 numberBuffers = 0;
        u32 numberSamplers = 0;
        for (const auto& binding : InBindings)
        {
            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                numberBuffers++;
            }
            else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                numberSamplers++;
            }
        }

        // Create descriptor pool
        std::vector<VkDescriptorPoolSize> poolSizes;

        if (numberBuffers > 0)
        {
            poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numberBuffers});
        }

        if (numberSamplers > 0)
        {
            poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numberSamplers});
        }

        VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.poolSizeCount  = static_cast<u32>(poolSizes.size());
        poolInfo.pPoolSizes     = poolSizes.data();
        poolInfo.maxSets        = 1;

        if (vkCreateDescriptorPool(InDevice->Handle, &poolInfo, nullptr, &internalState->descriptorPool) != VK_SUCCESS) {
            SERROR("Failed to create descriptor pool.");
            return;
        }

        VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool        = internalState->descriptorPool;
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = &InDescriptorSetLayout;

        // Allocate descriptor set
        VkResult res = vkAllocateDescriptorSets(InDevice->Handle, &allocInfo, &internalState->descriptorSet);
        if ( res != VK_SUCCESS) {
            SERROR("Failed to allocate descriptor set.");
            return;
        }
    }

    void VulkanDescriptorSet::BindDescriptor(VkCommandBuffer cmd) const
    {
        vkCmdBindDescriptorSets(cmd, pipelineBindPoint, pipelineLayout, setNumber, 1, &descriptorSet, 0, nullptr);
    }
} // Vk
} // Sogas
