#include "vulkan/vulkan_shader.h"
#include "vulkan/vulkan_device.h"
#include <spirv/src/spirv_cross.hpp>

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

static std::vector<u32> ReadShaderFile(const std::string& InFilename)
{
    FILE* file;
    fopen_s(&file, InFilename.c_str(), "rb");
    SASSERT_MSG(file, "No file provided with name '%s'", InFilename.c_str());

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file) / sizeof(u32);
    rewind(file);
    std::vector<u32> buffer(fileSize);
    if (fread(buffer.data(), sizeof(u32), fileSize, file) != fileSize)
    {
        buffer.clear();
    }

    fclose(file);
    return buffer;
}

VulkanShader::~VulkanShader()
{
    vkDestroyShaderModule(device->Handle, shaderModule, nullptr);
    layoutBindingsPerSet->clear();
    pushConstantRanges.clear();
}

VkShaderModuleCreateInfo CompileShader(const char* code, u32 size, VkShaderStageFlagBits stage, std::string name)
{
    VkShaderModuleCreateInfo ShaderCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

    return ShaderCreateInfo;
}

ShaderStateHandle VulkanShader::Create(VulkanDevice* InDevice, const ShaderStateDescriptor& InDescriptor)
{
    ShaderStateHandle handle = {INVALID_ID};

    if (InDescriptor.stages_count == 0 || InDescriptor.stages == nullptr)
    {
        SERROR("Shader %s does not contain any stages.\n", InDescriptor.name);
        return handle;
    }

    handle.index = InDevice->shaders.ObtainResource();

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanShaderState* ShaderState   = static_cast<VulkanShaderState*>(InDevice->shaders.AccessResource(handle.index));
    ShaderState->bIsGraphicsPipeline = true;
    ShaderState->ActiveShaders       = 0;
    u32 CompiledShaders              = 0;

    for (CompiledShaders; CompiledShaders < InDescriptor.stages_count; ++CompiledShaders)
    {
        const ShaderStage& stage = InDescriptor.stages[CompiledShaders];

        if (stage.type == ShaderStageType::COMPUTE)
        {
            ShaderState->bIsGraphicsPipeline = false;
        }

        VkShaderModuleCreateInfo ShaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

        if (InDescriptor.spv_input)
        {
            ShaderModuleInfo.codeSize = stage.size;
            ShaderModuleInfo.pCode    = reinterpret_cast<const u32*>(stage.code);
        }
        else
        {
            //ShaderModuleInfo = CompileShader(stage.code, stage.code_size, stage.type, InDescriptor.name);
        }

        VkPipelineShaderStageCreateInfo ShaderStageInfo = ShaderState->ShaderStageInfo[CompiledShaders];
        memset(&ShaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
        ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.pName = "main";
        ShaderStageInfo.stage = ConvertShaderStage(stage.type);

        if (vkCreateShaderModule(InDevice->Handle, &ShaderModuleInfo, nullptr, &ShaderState->ShaderStageInfo[CompiledShaders].module) != VK_SUCCESS)
        {
            break;
        }

        // TODO Set resource name.
    }

    bool bCreationFailed = CompiledShaders != InDescriptor.stages_count;

    if (!bCreationFailed)
    {
        ShaderState->ActiveShaders = CompiledShaders;
        ShaderState->Name = InDescriptor.name;
    }
    else
    {
        for (u32 i = 0; i < ShaderState->ActiveShaders; ++i)
        {
            vkDestroyShaderModule(InDevice->Handle, ShaderState->ShaderStageInfo[i].module, nullptr);
        }

        SERROR("Failed to create shader %s.\n", InDescriptor.name);
    }

    return handle;
}

void VulkanShader::Create(const VulkanDevice* device, ShaderStageType InStage, std::string InFilename, Shader* OutShader)
{
    std::string name = InFilename;
    auto        code = ReadShaderFile(name);
    SASSERT(!code.empty());

    spirv_cross::Compiler comp(code);

    spirv_cross::ShaderResources shaderResources = comp.get_shader_resources();

    auto internalState       = std::make_shared<VulkanShader>();
    internalState->device    = device;
    OutShader->internalState = internalState;
    OutShader->stage         = InStage;

    for (const auto& resource : shaderResources.push_constant_buffers)
    {
        auto ranges = comp.get_active_buffer_ranges(resource.id);
        for (auto& range : ranges)
        {
            STRACE("%d %d %d", range.index, range.offset, range.range);
            VkPushConstantRange pushConstantRange = {};
            pushConstantRange.offset              = static_cast<u32>(range.offset);
            pushConstantRange.size                = static_cast<u32>(range.range);
            pushConstantRange.stageFlags          = ConvertShaderStage(InStage);

            internalState->pushConstantRanges.push_back(pushConstantRange);
        }
    }

    for (const auto& resource : shaderResources.uniform_buffers)
    {
        u32 set     = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 binding = comp.get_decoration(resource.id, spv::DecorationBinding);
        STRACE("Found UBO with name %s at set %d binding %d.", resource.name.c_str(), set, binding);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding                      = binding;
        layoutBinding.descriptorCount              = 1;
        layoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.stageFlags                   = ConvertShaderStage(InStage);

        internalState->layoutBindingsPerSet[set].push_back(layoutBinding);
    }

    for (const auto& resource : shaderResources.sampled_images)
    {
        u32 set     = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 binding = comp.get_decoration(resource.id, spv::DecorationBinding);
        STRACE("Found sample with name %s at set %d binding %d", resource.name.c_str(), set, binding);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding                      = binding;
        layoutBinding.descriptorCount              = 1;
        layoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.stageFlags                   = ConvertShaderStage(InStage);
        internalState->layoutBindingsPerSet[set].push_back(layoutBinding);
    }

    VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    shaderModuleInfo.codeSize                 = static_cast<u32>(code.size() * sizeof(u32));
    shaderModuleInfo.pCode                    = reinterpret_cast<const u32*>(code.data());

    if (vkCreateShaderModule(device->Handle, &shaderModuleInfo, nullptr, &internalState->shaderModule) != VK_SUCCESS)
    {
        SERROR("Failed to create %s shader module", InFilename);
        return;
    }
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
