#include "vulkan/vulkan_pipeline.h"
#include "renderpass.h"
#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_shader.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

constexpr static VkBlendOp ConvertBlendOperation(BlendOperation InOp)
{
    switch (InOp)
    {
        case BlendOperation::ADD:
            return VK_BLEND_OP_ADD;
        case BlendOperation::SUBSTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOperation::REVERSE_SUBSTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOperation::MIN:
            return VK_BLEND_OP_MIN;
        case BlendOperation::MAX:
            return VK_BLEND_OP_MAX;
    }

    return VK_BLEND_OP_MAX;
}

constexpr static VkCullModeFlagBits ConvertCullMode(CullMode InCullMode)
{
    switch (InCullMode)
    {
        default:
        case CullMode::NONE:
            return VK_CULL_MODE_NONE;
        case CullMode::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::BACK:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::FRONT_AND_BACK:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }
}

constexpr static VkFrontFace ConvertFrontFace(FrontFace InFrontFace)
{
    switch (InFrontFace)
    {
        case FrontFace::COUNTER_CLOCKWISE:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case FrontFace::CLOCKWISE:
            return VK_FRONT_FACE_CLOCKWISE;
    }

    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

static std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages(const PipelineDescriptor* desc)
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo;

    auto addShaderStage = [&](const Shader* shader)
    {
        auto internalState = std::static_pointer_cast<VulkanShader>(shader->internalState);
        if (internalState != nullptr)
        {
            VkPipelineShaderStageCreateInfo info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            info.module                          = internalState->shaderModule;
            info.pName                           = "main";
            info.stage                           = ConvertShaderStage(shader->stage);

            shaderStagesInfo.push_back(info);
        }
    };

    if (desc->vs != nullptr)
    {
        addShaderStage(desc->vs);
    }

    if (desc->ps != nullptr)
    {
        addShaderStage(desc->ps);
    }

    return shaderStagesInfo;
}

VulkanPipeline::~VulkanPipeline()
{
    Destroy();
}

PipelineHandle VulkanPipeline::Create(VulkanDevice* InDevice, const PipelineDescriptor& InDescriptor)
{
    PipelineHandle handle = {InDevice->pipelines.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    ShaderStateHandle shader_state_handle = VulkanShader::Create(InDevice, InDescriptor.shaders);

    if (shader_state_handle.index == INVALID_ID)
    {
        InDevice->pipelines.ReleaseResource(handle.index);
        handle.index = INVALID_ID;
        return handle;
    }

    VulkanPipeline*    pipeline     = InDevice->GetPipelineResource(handle);
    VulkanShaderState* shader_state = InDevice->GetShaderResource(shader_state_handle);

    pipeline->shader_state = shader_state_handle;

    VkDescriptorSetLayout layouts[MAX_DESCRIPTOR_SET_LAYOUTS];

    for (u32 i = 0; i < InDescriptor.active_layouts_count; ++i)
    {
        pipeline->descriptor_set_layout[i]        = InDevice->GetDescriptorSetLayoutResource(InDescriptor.descriptor_set_layout[i]);
        pipeline->descriptor_set_layout_handle[i] = InDescriptor.descriptor_set_layout[i];

        layouts[i] = pipeline->descriptor_set_layout[i]->descriptor_set_layout;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipeline_layout_info.pSetLayouts                = layouts;
    pipeline_layout_info.setLayoutCount             = InDescriptor.active_layouts_count;

    VkPipelineLayout pipeline_layout;
    vkcheck(vkCreatePipelineLayout(InDevice->Handle, &pipeline_layout_info, nullptr, &pipeline_layout));

    pipeline->pipelineLayout      = pipeline_layout;
    pipeline->active_layout_count = InDescriptor.active_layouts_count;

    if (shader_state->bIsGraphicsPipeline)
    {
        VkGraphicsPipelineCreateInfo pipeline_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

        pipeline_info.pStages    = shader_state->ShaderStageInfo;
        pipeline_info.stageCount = shader_state->ActiveShaders;

        pipeline_info.layout = pipeline_layout;

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

        VkVertexInputAttributeDescription vertex_attributes[8];
        if (InDescriptor.vertexInputState.vertex_attributes_count)
        {
            for (u32 i = 0; i < InDescriptor.vertexInputState.vertex_attributes_count; ++i)
            {
                const VertexAttribute& vertex_attribute = InDescriptor.vertexInputState.vertex_attribute[i];
                vertex_attributes[i]                    = {vertex_attribute.location, vertex_attribute.binding, ConvertVertexFormat(vertex_attribute.format), vertex_attribute.offset};
            }

            vertex_input_info.vertexAttributeDescriptionCount = InDescriptor.vertexInputState.vertex_attributes_count;
            vertex_input_info.pVertexAttributeDescriptions    = vertex_attributes;
        }
        else
        {
            vertex_input_info.vertexAttributeDescriptionCount = 0;
            vertex_input_info.pVertexAttributeDescriptions    = nullptr;
        }

        VkVertexInputBindingDescription vertex_bindings[8];
        if (InDescriptor.vertexInputState.vertex_streams_count)
        {
            for (u32 i = 0; i < InDescriptor.vertexInputState.vertex_streams_count; ++i)
            {
                const auto&       vertex_binding = InDescriptor.vertexInputState.vertex_stream[i];
                VkVertexInputRate input_rate     = vertex_binding.input_rate == VertexInputRate::PER_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
                vertex_bindings[i]               = {vertex_binding.binding, vertex_binding.stride, input_rate};
            }

            vertex_input_info.vertexBindingDescriptionCount = InDescriptor.vertexInputState.vertex_streams_count;
            vertex_input_info.pVertexBindingDescriptions    = vertex_bindings;
        }
        else
        {
            vertex_input_info.vertexBindingDescriptionCount = 0;
            vertex_input_info.pVertexBindingDescriptions    = nullptr;
        }

        pipeline_info.pVertexInputState = &vertex_input_info;

        VkPipelineInputAssemblyStateCreateInfo inpute_assembly_info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inpute_assembly_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inpute_assembly_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachments[8];
        if (InDescriptor.blendState.ActiveStates)
        {
            for (size_t i = 0; i < InDescriptor.blendState.ActiveStates; ++i)
            {
                const BlendState& blend_state = InDescriptor.blendState.BlendStates[i];

                color_blend_attachments[i].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachments[i].blendEnable         = blend_state.BlendEnabled ? VK_TRUE : VK_FALSE;
                color_blend_attachments[i].srcColorBlendFactor = blend_state.SourceColor == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                color_blend_attachments[i].dstColorBlendFactor = blend_state.DestinationColor == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                color_blend_attachments[i].colorBlendOp        = ConvertBlendOperation(blend_state.ColorOperation);

                if (blend_state.SeparateBlend)
                {
                    color_blend_attachments[i].srcAlphaBlendFactor = blend_state.SourceAlpha == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                    color_blend_attachments[i].dstAlphaBlendFactor = blend_state.DestinationAlpha == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                    color_blend_attachments[i].alphaBlendOp        = ConvertBlendOperation(blend_state.AlphaOperation);
                }
                else
                {
                    color_blend_attachments[i].srcAlphaBlendFactor = blend_state.SourceColor == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                    color_blend_attachments[i].dstAlphaBlendFactor = blend_state.DestinationColor == BlendFactor::ONE ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
                    color_blend_attachments[i].alphaBlendOp        = ConvertBlendOperation(blend_state.ColorOperation);
                }
            }
        }
        else
        {
            color_blend_attachments[0]                = {};
            color_blend_attachments[0].blendEnable    = VK_FALSE;
            color_blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo blend_state_info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend_state_info.logicOpEnable     = VK_TRUE;
        blend_state_info.logicOp           = VK_LOGIC_OP_COPY;
        blend_state_info.attachmentCount   = InDescriptor.blendState.ActiveStates;
        blend_state_info.pAttachments      = color_blend_attachments;
        blend_state_info.blendConstants[0] = 0.0f;
        blend_state_info.blendConstants[1] = 0.0f;
        blend_state_info.blendConstants[2] = 0.0f;
        blend_state_info.blendConstants[3] = 0.0f;

        pipeline_info.pColorBlendState = &blend_state_info;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth_stencil_info.depthWriteEnable                      = InDescriptor.depthStencilState.depthTestEnabled ? VK_TRUE : VK_FALSE;
        depth_stencil_info.stencilTestEnable                     = InDescriptor.depthStencilState.stencilTestEnabled ? VK_TRUE : VK_FALSE;
        depth_stencil_info.depthCompareOp                        = ConvertCompareOperation(InDescriptor.depthStencilState.compareOp);

        if (InDescriptor.depthStencilState.stencilTestEnabled)
        {
            // TODO add stencil
            SASSERT(false);
        }

        pipeline_info.pDepthStencilState = &depth_stencil_info;

        VkPipelineMultisampleStateCreateInfo multisampling_state_info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisampling_state_info.sampleShadingEnable   = VK_FALSE;
        multisampling_state_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling_state_info.minSampleShading      = 1.0f;
        multisampling_state_info.pSampleMask           = nullptr;
        multisampling_state_info.alphaToCoverageEnable = VK_FALSE;
        multisampling_state_info.alphaToOneEnable      = VK_FALSE;

        pipeline_info.pMultisampleState = &multisampling_state_info;

        VkPipelineRasterizationStateCreateInfo rasterizer_info = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizer_info.depthClampEnable                       = VK_FALSE;
        rasterizer_info.rasterizerDiscardEnable                = VK_FALSE;
        rasterizer_info.polygonMode                            = VK_POLYGON_MODE_FILL;
        rasterizer_info.lineWidth                              = 1.0f;
        rasterizer_info.cullMode                               = ConvertCullMode(InDescriptor.rasterizationState.cull_mode);
        rasterizer_info.frontFace                              = ConvertFrontFace(InDescriptor.rasterizationState.front_face);
        rasterizer_info.depthBiasEnable                        = VK_FALSE;
        rasterizer_info.depthBiasConstantFactor                = 0.0f;
        rasterizer_info.depthBiasClamp                         = 0.0f;
        rasterizer_info.depthBiasSlopeFactor                   = 0.0f;

        pipeline_info.pRasterizationState = &rasterizer_info;

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<f32>(640);
        viewport.height   = static_cast<f32>(480);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {640, 480};

        VkPipelineViewportStateCreateInfo viewport_info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport_info.viewportCount = 1;
        viewport_info.pViewports    = &viewport;
        viewport_info.scissorCount  = 1;
        viewport_info.pScissors     = &scissor;

        pipeline_info.pViewportState = &viewport_info;

        pipeline_info.renderPass = InDevice->GetVulkanRenderPass(InDescriptor.render_pass, InDescriptor.name);

        VkDynamicState                   dynamic_state[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic_state_info{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic_state_info.dynamicStateCount = 2;
        dynamic_state_info.pDynamicStates    = dynamic_state;

        pipeline_info.pDynamicState = &dynamic_state_info;

        vkCreateGraphicsPipelines(InDevice->Handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline->pipeline);
        pipeline->bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    }
    else
    {
        VkComputePipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

        pipeline_info.stage  = shader_state->ShaderStageInfo[0];
        pipeline_info.layout = pipeline_layout;

        vkCreateComputePipelines(InDevice->Handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline->pipeline);
        pipeline->bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    return handle;
}

void VulkanPipeline::Create(VulkanDevice*             device,
                            const PipelineDescriptor* desc,
                            Pipeline*                 pipeline,
                            Renderer::RenderPass*     renderpass)
{
    SASSERT(device);
    SASSERT(desc);

    auto internalState = std::static_pointer_cast<VulkanPipeline>(pipeline->internalState);
    if (internalState == nullptr)
    {
        internalState = std::make_shared<VulkanPipeline>();
    }

    internalState->device = device;

    pipeline->internalState = internalState;
    pipeline->descriptor    = *desc;

    auto shaderStages = GetShaderStages(desc);

    std::array<VkDynamicState, 2> dynamicState = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount                = static_cast<u32>(dynamicState.size());
    dynamicStateInfo.pDynamicStates                   = dynamicState.data();

    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputStateInfo.vertexBindingDescriptionCount   = 0;
    vertexInputStateInfo.pVertexBindingDescriptions      = nullptr;
    vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateInfo.pVertexAttributeDescriptions    = nullptr;

    if (!pipeline->descriptor.vertexDeclaration.empty())
    {
        VkVertexInputBindingDescription vertexBinding = {};
        vertexBinding.binding                         = 0;
        vertexBinding.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexBinding.stride                          = sizeof(VulkanVertex);

        vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateInfo.pVertexBindingDescriptions    = &vertexBinding;
        vertexInputStateInfo.vertexAttributeDescriptionCount =
          static_cast<u32>(GetVertexDeclaration(pipeline->descriptor.vertexDeclaration)->size);
        vertexInputStateInfo.pVertexAttributeDescriptions =
          GetVertexDeclaration(pipeline->descriptor.vertexDeclaration)->layout;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 480.0f;
    viewport.width    = 640.0f;
    viewport.height   = -480.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissors{};
    scissors.extent = {640, 480};
    scissors.offset = {0, 0};

    VkPipelineViewportStateCreateInfo viewportStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportStateInfo.viewportCount                     = 1;
    viewportStateInfo.pViewports                        = &viewport;
    viewportStateInfo.scissorCount                      = 1;
    viewportStateInfo.pScissors                         = &scissors;

    // TODO populate with information from pipeline descriptor.
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationStateInfo.depthClampEnable        = VK_FALSE;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo.lineWidth               = 1.0f;
    rasterizationStateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizationStateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateInfo.depthBiasEnable         = VK_FALSE;

    // VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
    //     VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    // if (desc->depthStencilState != nullptr)
    // {
    //     depthStencilStateInfo.depthTestEnable   = desc->depthStencilState.depthTestEnabled ? VK_TRUE : VK_FALSE;
    //     depthStencilStateInfo.depthWriteEnable  = desc->depthStencilState.writeDepthEnabled ? VK_TRUE : VK_FALSE;
    //     depthStencilStateInfo.stencilTestEnable = desc->depthStencilState.stencilTestEnabled ? VK_TRUE : VK_FALSE;
    //     depthStencilStateInfo.depthBoundsTestEnable =
    //         desc->depthStencilState->depthBoundTestEnabled ? VK_TRUE : VK_FALSE;
    //     depthStencilStateInfo.depthCompareOp = ConvertCompareOperation(desc->depthStencilState->compareOp);
    //     depthStencilStateInfo.minDepthBounds = desc->depthStencilState->minDepthBound;
    //     depthStencilStateInfo.maxDepthBounds = desc->depthStencilState->maxDepthBound;
    //     depthStencilStateInfo.front          = {}; // TODO check them out
    //     depthStencilStateInfo.back           = {};
    // }

    VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleStateInfo.sampleShadingEnable  = VK_FALSE;
    multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments    = &colorBlendAttachment;
    colorBlendStateInfo.logicOpEnable   = VK_FALSE;

    auto ps = VulkanShader::ToInternal(pipeline->descriptor.ps);
    auto vs = VulkanShader::ToInternal(pipeline->descriptor.vs);

    for (u32 i = 0; i < 8; i++)
    {
        size_t numberOfBindings = ps->layoutBindingsPerSet[i].size() + vs->layoutBindingsPerSet[i].size();
        if (numberOfBindings > 0)
        {
            internalState->descriptorSetLayoutBindingsPerSet[i].reserve(numberOfBindings);
            internalState->descriptorSetLayoutBindingsPerSet[i].insert(
              internalState->descriptorSetLayoutBindingsPerSet[i].end(),
              vs->layoutBindingsPerSet[i].begin(),
              vs->layoutBindingsPerSet[i].end());
            internalState->descriptorSetLayoutBindingsPerSet[i].insert(
              internalState->descriptorSetLayoutBindingsPerSet[i].end(),
              ps->layoutBindingsPerSet[i].begin(),
              ps->layoutBindingsPerSet[i].end());

            VkDescriptorSetLayoutCreateInfo info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            info.bindingCount                    = static_cast<u32>(internalState->descriptorSetLayoutBindingsPerSet[i].size());
            info.pBindings                       = internalState->descriptorSetLayoutBindingsPerSet[i].data();

            VkDescriptorSetLayout descriptorSetLayout;
            VkResult              res = vkCreateDescriptorSetLayout(device->Handle, &info, nullptr, &descriptorSetLayout);
            if (res != VK_SUCCESS)
            {
                SERROR("Failed to create descriptor set layout!");
                return;
            }

            internalState->descriptorSetLayouts.push_back(descriptorSetLayout);
        }
    }

    std::vector<VkPushConstantRange> ranges;
    if (!vs->pushConstantRanges.empty())
    {
        VkPushConstantRange vertexShaderRange{};
        for (auto& pc : vs->pushConstantRanges)
        {
            vertexShaderRange.size += pc.size;
        }
        vertexShaderRange.offset     = vs->pushConstantRanges.at(0).offset;
        vertexShaderRange.stageFlags = vs->pushConstantRanges.at(0).stageFlags;
        ranges.push_back(vertexShaderRange);
    }

    if (!ps->pushConstantRanges.empty())
    {
        VkPushConstantRange pixelShaderRange{};
        for (auto& pc : ps->pushConstantRanges)
        {
            pixelShaderRange.size += pc.size;
        }
        pixelShaderRange.offset     = ps->pushConstantRanges.at(0).offset;
        pixelShaderRange.stageFlags = ps->pushConstantRanges.at(0).stageFlags;
        ranges.push_back(pixelShaderRange);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.pushConstantRangeCount     = static_cast<u32>(ranges.size());
    pipelineLayoutInfo.pPushConstantRanges        = ranges.data();
    pipelineLayoutInfo.setLayoutCount             = static_cast<u32>(internalState->descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts                = internalState->descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(device->Handle, &pipelineLayoutInfo, nullptr, &internalState->pipelineLayout) !=
        VK_SUCCESS)
    {
        SFATAL("Failed to create graphics pipeline layout!");
        return;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount                   = static_cast<u32>(shaderStages.size());
    pipelineInfo.pStages                      = shaderStages.data();
    pipelineInfo.layout                       = internalState->pipelineLayout;
    pipelineInfo.pColorBlendState             = &colorBlendStateInfo;
    pipelineInfo.pDynamicState                = &dynamicStateInfo;
    //pipelineInfo.pDepthStencilState           = &depthStencilStateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pMultisampleState   = &multisampleStateInfo;
    pipelineInfo.pRasterizationState = &rasterizationStateInfo;
    pipelineInfo.pTessellationState  = nullptr;
    pipelineInfo.pVertexInputState   = &vertexInputStateInfo;
    pipelineInfo.pViewportState      = &viewportStateInfo;
    pipelineInfo.subpass             = 0;
    pipelineInfo.renderPass          = VulkanRenderPass::ToInternal(renderpass)->renderpass;

    if (vkCreateGraphicsPipelines(device->Handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &internalState->pipeline) !=
        VK_SUCCESS)
    {
        SFATAL("Failed to create graphics pipeline.");
        return;
    }

    internalState->CreateDescriptorSets();
}

void VulkanPipeline::CreateDescriptorSets()
{
    for (u32 frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        u32 i = 0;
        for (auto& descriptorSetLayout : descriptorSetLayouts)
        {
            DescriptorSet descriptorSet;
            VulkanDescriptorSet::Create(device,
                                        &descriptorSet,
                                        descriptorSetLayout,
                                        pipelineLayout,
                                        descriptorSetLayoutBindingsPerSet[i],
                                        i);
            descriptorSets[frameIndex].push_back(descriptorSet);
            i++;
        }
    }
}

void VulkanPipeline::Destroy()
{
    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device->Handle, descriptorSetLayout, nullptr);
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        for (auto descriptorSet : descriptorSets[i])
        {
            descriptorSet.Destroy();
        }
    }

    vkDestroyPipeline(device->Handle, pipeline, nullptr);
    vkDestroyPipelineLayout(device->Handle, pipelineLayout, nullptr);

    pipeline       = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
