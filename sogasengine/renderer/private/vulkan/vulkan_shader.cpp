#include "vulkan/vulkan_shader.h"
#include "vulkan/vulkan_device.h"
#include <spirv/src/spirv_cross.hpp>

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

std::vector<i8> VulkanShader::ReadBinaryShaderFile(const std::string& InFilename)
{
    FILE* file;
    fopen_s(&file, InFilename.c_str(), "rb");
    SASSERT_MSG(file, "No file provided with name '%s'", InFilename.c_str());

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    std::vector<i8> buffer(fileSize);
    if (fread(buffer.data(), sizeof(i8), fileSize, file) != fileSize)
    {
        buffer.clear();
    }

    fclose(file);
    return buffer;
}

char* VulkanShader::ReadShaderFile(const std::string& InFilename, u32& OutSize)
{
    char* buffer = nullptr;
    FILE* file;
    fopen_s(&file, InFilename.c_str(), "r");
    SASSERT_MSG(file != nullptr, "No file provided with name '%s'", InFilename.c_str());

    if (file)
    {
        // Get file size
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        rewind(file);

        buffer = static_cast<char*>(malloc(fileSize + 1));

        auto read = fread(buffer, sizeof(i8), fileSize, file);
        //SASSERT_MSG(read == fileSize, "fread did not read expected number of characters.");

        buffer[read] = 0;

        OutSize = read;
       
        fclose(file);
        return buffer;
    }

    return nullptr;
}

static std::string add_shader_extension(VkShaderStageFlagBits stage)
{
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return ".vert";
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return ".frag";
    case VK_SHADER_STAGE_COMPUTE_BIT:
        return ".comp";
    default:
        return "";
    }
}

VkShaderModuleCreateInfo CompileShader(const char* code, u32 size, VkShaderStageFlagBits stage, std::string name)
{
    VkShaderModuleCreateInfo ShaderCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

    // Create dummy file
    std::string temporal_shader_name = "temporal" + add_shader_extension(stage);
    FILE* temporal_shader_file = fopen(temporal_shader_name.c_str(), "w");
    fwrite( code, size, 1, temporal_shader_file);
    fclose(temporal_shader_file);

    std::string output_name = temporal_shader_name + ".spv";
    std::string command = std::string("glslc.exe ") + temporal_shader_name + " -o " + output_name;

    system(command.c_str());

    auto binary_data = VulkanShader::ReadBinaryShaderFile(output_name);

    ShaderCreateInfo.codeSize = static_cast<u32>(binary_data.size());
    ShaderCreateInfo.pCode = reinterpret_cast<u32*>(binary_data.data());

    i32 result = remove(temporal_shader_name.c_str());

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
            ShaderModuleInfo = CompileShader(stage.code, stage.size, ConvertShaderStage(stage.type), InDescriptor.name);
        }

        VkPipelineShaderStageCreateInfo& ShaderStageInfo = ShaderState->ShaderStageInfo[CompiledShaders];
        memset(&ShaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
        ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.pName = "main";
        ShaderStageInfo.stage = ConvertShaderStage(stage.type);

        VkShaderModule module;
        auto ok = vkCreateShaderModule(InDevice->Handle, &ShaderModuleInfo, nullptr, &module);

        ShaderState->ShaderStageInfo[CompiledShaders].module = module;
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

// void VulkanShader::Create(const VulkanDevice* device, ShaderStageType InStage, std::string InFilename, Shader* OutShader)
// {
//     std::string name = InFilename;
//     auto        code = ReadShaderFile(name);
//     SASSERT(!code.empty());

//     spirv_cross::Compiler comp(code);

//     spirv_cross::ShaderResources shaderResources = comp.get_shader_resources();

//     auto internalState       = std::make_shared<VulkanShader>();
//     internalState->device    = device;
//     OutShader->internalState = internalState;
//     OutShader->stage         = InStage;

//     for (const auto& resource : shaderResources.push_constant_buffers)
//     {
//         auto ranges = comp.get_active_buffer_ranges(resource.id);
//         for (auto& range : ranges)
//         {
//             STRACE("%d %d %d", range.index, range.offset, range.range);
//             VkPushConstantRange pushConstantRange = {};
//             pushConstantRange.offset              = static_cast<u32>(range.offset);
//             pushConstantRange.size                = static_cast<u32>(range.range);
//             pushConstantRange.stageFlags          = ConvertShaderStage(InStage);

//             internalState->pushConstantRanges.push_back(pushConstantRange);
//         }
//     }

//     for (const auto& resource : shaderResources.uniform_buffers)
//     {
//         u32 set     = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
//         u32 binding = comp.get_decoration(resource.id, spv::DecorationBinding);
//         STRACE("Found UBO with name %s at set %d binding %d.", resource.name.c_str(), set, binding);

//         VkDescriptorSetLayoutBinding layoutBinding = {};
//         layoutBinding.binding                      = binding;
//         layoutBinding.descriptorCount              = 1;
//         layoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         layoutBinding.stageFlags                   = ConvertShaderStage(InStage);

//         internalState->layoutBindingsPerSet[set].push_back(layoutBinding);
//     }

//     for (const auto& resource : shaderResources.sampled_images)
//     {
//         u32 set     = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
//         u32 binding = comp.get_decoration(resource.id, spv::DecorationBinding);
//         STRACE("Found sample with name %s at set %d binding %d", resource.name.c_str(), set, binding);

//         VkDescriptorSetLayoutBinding layoutBinding = {};
//         layoutBinding.binding                      = binding;
//         layoutBinding.descriptorCount              = 1;
//         layoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         layoutBinding.stageFlags                   = ConvertShaderStage(InStage);
//         internalState->layoutBindingsPerSet[set].push_back(layoutBinding);
//     }

//     VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
//     shaderModuleInfo.codeSize                 = static_cast<u32>(code.size() * sizeof(u32));
//     shaderModuleInfo.pCode                    = reinterpret_cast<const u32*>(code.data());

//     if (vkCreateShaderModule(device->Handle, &shaderModuleInfo, nullptr, &internalState->shaderModule) != VK_SUCCESS)
//     {
//         SERROR("Failed to create %s shader module", InFilename);
//         return;
//     }
// }

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
