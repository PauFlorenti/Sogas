#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_pipeline.h"

namespace Sogas
{
namespace Vk
{
    void VulkanPipeline::Create(const VulkanDevice* device, const PipelineDescriptor* desc, Pipeline* pipeline)
    {
        SASSERT(device);
        SASSERT(desc);

        auto internalState = std::static_pointer_cast<VulkanPipeline*>(pipeline->internalState);
        if (internalState == nullptr)
        {
            internalState = std::make_shared<VulkanPipeline*>();
        }

        pipeline->internalState = internalState;

        VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

        if (vkCreateGraphicsPipelines(device->Handle, 0, 1, &pipelineInfo, nullptr, &internalState->handle) != VK_SUCCESS) {
            SFATAL("Failed to create graphics pipeline!");
            return;
        }
    }
} // Vk
} // Sogas