#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_pipeline.h"
#include "render/vulkan/vulkan_renderpass.h"
#include "render/vulkan/vulkan_shader.h"

namespace Sogas
{
namespace Vk
{

    static std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages(const PipelineDescriptor* desc)
    {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo;

        auto addShaderStage = [&](Shader* shader) {
            auto internalState = std::static_pointer_cast<VulkanShader>(shader->internalState);
            if (internalState != nullptr)
            {
                VkPipelineShaderStageCreateInfo info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                info.module = internalState->shaderModule;
                info.pName  = "main";
                info.stage  = ConvertShaderStage(shader->stage);

                shaderStagesInfo.push_back(info);
            }
        };

        if (desc->vs != nullptr) {
            addShaderStage(desc->vs);
        }

        if (desc->ps != nullptr) {
            addShaderStage(desc->ps);
        }

        return shaderStagesInfo;
    }

    void VulkanPipeline::Create(const VulkanDevice* device, const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass)
    {
        SASSERT(device);
        SASSERT(desc);

        auto internalState = std::static_pointer_cast<VulkanPipeline>(pipeline->internalState);
        if (internalState == nullptr)
        {
            internalState = std::make_shared<VulkanPipeline>();
        }

        pipeline->internalState = internalState;
        pipeline->descriptor    = *desc;

        auto shaderStages = GetShaderStages(desc);

        std::array<VkDynamicState, 2> dynamicState = {
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_VIEWPORT
        };

        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicStateInfo.dynamicStateCount  = static_cast<u32>(dynamicState.size());
        dynamicStateInfo.pDynamicStates     = dynamicState.data();
        

        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInputStateInfo.vertexBindingDescriptionCount   = 0;
        vertexInputStateInfo.pVertexBindingDescriptions      = nullptr;
        vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateInfo.pVertexAttributeDescriptions    = nullptr;

        if (!pipeline->descriptor.vertexDeclaration.empty())
        {
            VkVertexInputBindingDescription vertexBinding = {};
            vertexBinding.binding   = 0;
            vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertexBinding.stride    = sizeof(VulkanVertex);

            vertexInputStateInfo.vertexBindingDescriptionCount   = 1;
            vertexInputStateInfo.pVertexBindingDescriptions      = &vertexBinding;
            vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<u32>(GetVertexDeclaration(pipeline->descriptor.vertexDeclaration)->size);
            vertexInputStateInfo.pVertexAttributeDescriptions    = GetVertexDeclaration(pipeline->descriptor.vertexDeclaration)->layout;
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssemblyInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 480.0f;
        viewport.width      = 640.0f;
        viewport.height     = -480.0f;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissors{};
        scissors.extent = {640, 480};
        scissors.offset = {0, 0};

        VkPipelineViewportStateCreateInfo viewportStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports    = &viewport;
        viewportStateInfo.scissorCount  = 1;
        viewportStateInfo.pScissors     = &scissors;

        VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizationStateInfo.depthClampEnable         = VK_FALSE;
        rasterizationStateInfo.rasterizerDiscardEnable  = VK_FALSE;
        rasterizationStateInfo.polygonMode              = VK_POLYGON_MODE_FILL;
        rasterizationStateInfo.lineWidth                = 1.0f;
        rasterizationStateInfo.cullMode                 = VK_CULL_MODE_BACK_BIT;
        rasterizationStateInfo.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateInfo.depthBiasEnable          = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisampleStateInfo.sampleShadingEnable  = VK_FALSE;
        multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask =   VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable    = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments    = &colorBlendAttachment;
        colorBlendStateInfo.logicOpEnable   = VK_FALSE;

        std::vector<VkPushConstantRange> pushConstantRanges;
        for (auto& pc : pipeline->descriptor.pushConstantDesc)
        {
            VkPushConstantRange pcRange = {};
            pcRange.offset     = pc.offset;
            pcRange.size       = pc.size;
            pcRange.stageFlags = ConvertShaderStage(pc.stage);

            pushConstantRanges.push_back(pcRange);
        }

        for (auto& ds : pipeline->descriptor.descriptorSetDesc)
        {
            VkDescriptorSetLayoutBinding binding;
            binding.binding         = ds.binding;
            binding.descriptorCount = 1;
            binding.descriptorType  = ConvertDescriptorType(ds.uniformType);
            binding.stageFlags      = ConvertShaderStage(ds.stage);

            internalState->descriptorSetLayoutBindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        info.bindingCount   = static_cast<u32>(internalState->descriptorSetLayoutBindings.size());
        info.pBindings      = internalState->descriptorSetLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(device->Handle, &info, nullptr, &internalState->descriptorSetLayout) != VK_SUCCESS) {
            SERROR("Failed to create descriptor set layout!");
            return;
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutInfo.pushConstantRangeCount   = static_cast<u32>(pushConstantRanges.size());
        pipelineLayoutInfo.pPushConstantRanges      = pushConstantRanges.data();
        pipelineLayoutInfo.setLayoutCount           = 1;
        pipelineLayoutInfo.pSetLayouts              = &internalState->descriptorSetLayout;

        if (vkCreatePipelineLayout(device->Handle, &pipelineLayoutInfo, nullptr, &internalState->pipelineLayout) != VK_SUCCESS) {
            SFATAL("Failed to create graphics pipeline layout!");
            return;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipelineInfo.stageCount             = static_cast<u32>(shaderStages.size());
        pipelineInfo.pStages                = shaderStages.data();
        pipelineInfo.layout                 = internalState->pipelineLayout;
        pipelineInfo.pColorBlendState       = &colorBlendStateInfo;
        pipelineInfo.pDynamicState          = &dynamicStateInfo;
        pipelineInfo.pDepthStencilState     = nullptr;
        pipelineInfo.pInputAssemblyState    = &inputAssemblyInfo;
        pipelineInfo.pMultisampleState      = &multisampleStateInfo;
        pipelineInfo.pRasterizationState    = &rasterizationStateInfo;
        pipelineInfo.pTessellationState     = nullptr;
        pipelineInfo.pVertexInputState      = &vertexInputStateInfo;
        pipelineInfo.pViewportState         = &viewportStateInfo;
        pipelineInfo.subpass                = 0;
        pipelineInfo.renderPass             = std::static_pointer_cast<VulkanRenderPass>(renderpass->internalState)->renderpass;

        if (vkCreateGraphicsPipelines(device->Handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &internalState->handle) != VK_SUCCESS) {
            SFATAL("Failed to create graphics pipeline.");
            return;
        }
    }

} // Vk
} // Sogas
