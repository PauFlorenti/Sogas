#include "render/vulkan/vulkan_device.h"
#include "render/vulkan/vulkan_shader.h"

namespace Sogas
{
namespace Vk
{
    static std::vector<char> ReadShaderFile(const std::string &InFilename)
    {
        std::ifstream file(InFilename, std::ios::ate | std::ios::binary);
        SASSERT_MSG(file, "No file provided with name '%s'", InFilename.c_str());
        SASSERT_MSG(file.is_open(), "Failed to open file '%s'", InFilename.c_str());

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    void VulkanShader::Create(const VulkanDevice* device, ShaderStage stage, const char* InFilename, Shader* OutShader)
    {
        std::string name = InFilename;
        std::string filename = "../../data/shaders/" + name;
        auto code = ReadShaderFile(filename);
        SASSERT(!code.empty());

        auto internalState          = std::make_shared<VulkanShader>();
        OutShader->internalState    = internalState;
        OutShader->stage            = stage;

        VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleInfo.codeSize   = static_cast<u32>(code.size());
        shaderModuleInfo.pCode      = reinterpret_cast<const u32 *>(code.data());
        
        if (vkCreateShaderModule(device->Handle, &shaderModuleInfo, nullptr, &internalState->shaderModule) != VK_SUCCESS) {
            SERROR("Failed to create %s shader module", InFilename);
            return;
        }
    }

} // Vk
} // Sogas
