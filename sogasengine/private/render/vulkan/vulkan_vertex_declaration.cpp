#include "render/vulkan/vulkan_vertex_declaration.h"

namespace Sogas {
namespace Vk
{
    static VkVertexInputAttributeDescription layoutPos[] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}
    };
    CVulkanVertexDeclaration vtx_decl_pos("Pos", layoutPos, 1);

    static VkVertexInputAttributeDescription layoutPosColor[] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(f32) * 3}
    };
    CVulkanVertexDeclaration vtx_decl_pos_color("PosColor", layoutPosColor, 2);

    CVulkanVertexDeclaration::CVulkanVertexDeclaration(const char* InName, const VkVertexInputAttributeDescription* InLayout, const u32 InSize)
        : name(InName), layout(InLayout), size(InSize)
    { };

    const CVulkanVertexDeclaration* GetVertexDeclaration(const std::string& InName)
    {
        if( InName == vtx_decl_pos.name )
            return &vtx_decl_pos;
        if( InName == vtx_decl_pos_color.name )
            return &vtx_decl_pos_color;
        return nullptr;
    }
} // Vk
} // Sogas
