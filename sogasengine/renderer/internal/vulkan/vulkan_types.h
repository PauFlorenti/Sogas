#pragma once

#include "render_device.h"
#include "texture.h"
#include "vulkan_vertex_declaration.h"
#include <vulkan/vulkan.h>

static const u32 MAX_FRAMES_IN_FLIGHT = 2;

namespace Sogas
{
namespace Renderer
{

namespace Vk
{
struct VulkanVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color;
};

struct ConstantsCamera
{
    glm::mat4 camera_projection;
    glm::mat4 camera_view;
    glm::mat4 camera_view_projection;
    glm::mat4 camera_inverse_view_projection;

    // Camera forward
    // Camera z far
    // Camera position
    // Camera Right
    // Camera Up
};

struct ConstantsObject
{
    glm::mat4 world;
    glm::vec4 color;
};

struct VulkanMesh
{
    u32 vertexCount  = 0;
    u32 vertexSize   = 0;
    u32 vertexOffset = 0;
    u32 indexCount   = 0;
    u32 indexOffset  = 0;

    u32 id = INVALID_ID;

    // TODO Make a buffer class
    VkBuffer       vertexBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    VkBuffer       indexBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
};

using namespace Sogas::Renderer;

constexpr VkImageType GetImageType(TextureDescriptor::TextureType type)
{
    switch (type)
    {
        case TextureDescriptor::TextureType::TEXTURE_TYPE_1D:
            return VK_IMAGE_TYPE_1D;
        default:
        case TextureDescriptor::TextureType::TEXTURE_TYPE_2D:
            return VK_IMAGE_TYPE_2D;
        case TextureDescriptor::TextureType::TEXTURE_TYPE_3D:
            return VK_IMAGE_TYPE_3D;
    }
}

constexpr u32 GetFormatStride(Format format)
{
    switch (format)
    {
        case Format::R32G32B32A32_SFLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 16u;
        case Format::R32G32B32_SFLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 12u;
        case Format::R64_UINT:
        case Format::R64_SINT:
        case Format::R64_SFLOAT:
        case Format::R32G32_SFLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
        case Format::R16G16B16A16_SFLOAT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_SINT:
        case Format::R16G16B16A16_UINT:
            return 8u;
        case Format::R16G16B16_SFLOAT:
        case Format::R16G16B16_SNORM:
        case Format::R16G16B16_UNORM:
        case Format::R16G16B16_SINT:
        case Format::R16G16B16_UINT:
            return 6u;
        case Format::R32_SFLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
        case Format::R16G16_SFLOAT:
        case Format::R16G16_SNORM:
        case Format::R16G16_UNORM:
        case Format::R16G16_SINT:
        case Format::R16G16_UINT:
        case Format::R8G8B8A8_SRGB:
        case Format::R8G8B8A8_SSCALED:
        case Format::R8G8B8A8_USCALED:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_SINT:
        case Format::R8G8B8A8_UINT:
        case Format::B8G8R8A8_SRGB:
        case Format::B8G8R8A8_SSCALED:
        case Format::B8G8R8A8_USCALED:
        case Format::B8G8R8A8_SNORM:
        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8A8_SINT:
        case Format::B8G8R8A8_UINT:
            return 4u;
        case Format::R8G8B8_SRGB:
        case Format::R8G8B8_SSCALED:
        case Format::R8G8B8_USCALED:
        case Format::R8G8B8_SNORM:
        case Format::R8G8B8_UNORM:
        case Format::R8G8B8_SINT:
        case Format::R8G8B8_UINT:
        case Format::B8G8R8_SRGB:
        case Format::B8G8R8_SSCALED:
        case Format::B8G8R8_USCALED:
        case Format::B8G8R8_SNORM:
        case Format::B8G8R8_UNORM:
        case Format::B8G8R8_SINT:
        case Format::B8G8R8_UINT:
            return 3u;
        case Format::R16_SFLOAT:
        case Format::R16_SNORM:
        case Format::R16_UNORM:
        case Format::R16_SINT:
        case Format::R16_UINT:
        case Format::R8G8_SRGB:
        case Format::R8G8_SSCALED:
        case Format::R8G8_USCALED:
        case Format::R8G8_SNORM:
        case Format::R8G8_UNORM:
        case Format::R8G8_SINT:
        case Format::R8G8_UINT:
            return 2u;
        case Format::R8_SRGB:
        case Format::R8_SSCALED:
        case Format::R8_USCALED:
        case Format::R8_SNORM:
        case Format::R8_UNORM:
        case Format::R8_SINT:
        case Format::R8_UINT:
            return 1u;
        default:
            return 0;
    }
}

constexpr VkFormat ConvertFormat(Format format)
{
    switch (format)
    {
        case Format::R32G32B32A32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case Format::R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case Format::R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case Format::R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case Format::R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case Format::R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case Format::R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case Format::R32_SINT:
            return VK_FORMAT_R32_SFLOAT;
        case Format::R32_UINT:
            return VK_FORMAT_R32_UINT;
        case Format::R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case Format::R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case Format::R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case Format::R16G16B16A16_SSCALED:
            return VK_FORMAT_R16G16B16A16_SSCALED;
        case Format::R16G16B16A16_USCALED:
            return VK_FORMAT_R16G16B16A16_USCALED;
        case Format::R16G16B16_SFLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case Format::R16G16B16_SINT:
            return VK_FORMAT_R16G16B16_SINT;
        case Format::R16G16B16_UINT:
            return VK_FORMAT_R16G16B16_UINT;
        case Format::R16G16B16_SNORM:
            return VK_FORMAT_R16G16B16_SNORM;
        case Format::R16G16B16_UNORM:
            return VK_FORMAT_R16G16B16_UNORM;
        case Format::R16G16B16_SSCALED:
            return VK_FORMAT_R16G16B16_SSCALED;
        case Format::R16G16B16_USCALED:
            return VK_FORMAT_R16G16B16_USCALED;
        case Format::R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case Format::R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case Format::R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case Format::R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case Format::R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case Format::R16G16_SSCALED:
            return VK_FORMAT_R16G16_SSCALED;
        case Format::R16G16_USCALED:
            return VK_FORMAT_R16G16_USCALED;
        case Format::R16_SFLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case Format::R16_SINT:
            return VK_FORMAT_R16_SINT;
        case Format::R16_UINT:
            return VK_FORMAT_R16_UINT;
        case Format::R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case Format::R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case Format::R16_SSCALED:
            return VK_FORMAT_R16_SSCALED;
        case Format::R16_USCALED:
            return VK_FORMAT_R16_USCALED;
        case Format::R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case Format::R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case Format::R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case Format::R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::R8G8B8A8_SSCALED:
            return VK_FORMAT_R8G8B8A8_SSCALED;
        case Format::R8G8B8A8_USCALED:
            return VK_FORMAT_R8G8B8A8_USCALED;
        case Format::B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::B8G8R8A8_SINT:
            return VK_FORMAT_B8G8R8A8_SINT;
        case Format::B8G8R8A8_UINT:
            return VK_FORMAT_B8G8R8A8_UINT;
        case Format::B8G8R8A8_SNORM:
            return VK_FORMAT_B8G8R8A8_SNORM;
        case Format::B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8A8_SSCALED:
            return VK_FORMAT_B8G8R8A8_SSCALED;
        case Format::B8G8R8A8_USCALED:
            return VK_FORMAT_B8G8R8A8_USCALED;
        case Format::R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case Format::R8G8B8_SINT:
            return VK_FORMAT_R8G8B8_SINT;
        case Format::R8G8B8_UINT:
            return VK_FORMAT_R8G8B8_UINT;
        case Format::R8G8B8_SNORM:
            return VK_FORMAT_R8G8B8_SNORM;
        case Format::R8G8B8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case Format::R8G8B8_SSCALED:
            return VK_FORMAT_R8G8B8_SSCALED;
        case Format::R8G8B8_USCALED:
            return VK_FORMAT_R8G8B8_USCALED;
        case Format::B8G8R8_SRGB:
            return VK_FORMAT_B8G8R8_SRGB;
        case Format::B8G8R8_SINT:
            return VK_FORMAT_B8G8R8_SINT;
        case Format::B8G8R8_UINT:
            return VK_FORMAT_B8G8R8_UINT;
        case Format::B8G8R8_SNORM:
            return VK_FORMAT_B8G8R8_SNORM;
        case Format::B8G8R8_UNORM:
            return VK_FORMAT_B8G8R8_UNORM;
        case Format::B8G8R8_SSCALED:
            return VK_FORMAT_B8G8R8_SSCALED;
        case Format::B8G8R8_USCALED:
            return VK_FORMAT_B8G8R8_USCALED;
        case Format::R8G8_SRGB:
            return VK_FORMAT_R8G8_SRGB;
        case Format::R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case Format::R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case Format::R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case Format::R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case Format::R8G8_SSCALED:
            return VK_FORMAT_R8G8_SSCALED;
        case Format::R8G8_USCALED:
            return VK_FORMAT_R8G8_USCALED;
        case Format::R8_SRGB:
            return VK_FORMAT_R8_SRGB;
        case Format::R8_SINT:
            return VK_FORMAT_R8_SINT;
        case Format::R8_UINT:
            return VK_FORMAT_R8_UINT;
        case Format::R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case Format::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case Format::R8_SSCALED:
            return VK_FORMAT_R8_SSCALED;
        case Format::R8_USCALED:
            return VK_FORMAT_R8_USCALED;
        case Format::S8_UINT:
            return VK_FORMAT_S8_UINT;
        case Format::D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case Format::D32_UNORM_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D16_UNORM_S8_UINT:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        default:
            SERROR("Trying to convert a non-valid format.");
            return VK_FORMAT_UNDEFINED;
    }
}

constexpr Format ConvertFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return Format::R32G32B32A32_SFLOAT;
        case VK_FORMAT_R32G32B32A32_SINT:
            return Format::R32G32B32A32_SINT;
        case VK_FORMAT_R32G32B32A32_UINT:
            return Format::R32G32B32A32_UINT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return Format::R32G32B32_SFLOAT;
        case VK_FORMAT_R32G32B32_SINT:
            return Format::R32G32B32_SINT;
        case VK_FORMAT_R32G32B32_UINT:
            return Format::R32G32B32_UINT;
        case VK_FORMAT_R32G32_SFLOAT:
            return Format::R32G32_SFLOAT;
        case VK_FORMAT_R32G32_SINT:
            return Format::R32G32_SINT;
        case VK_FORMAT_R32G32_UINT:
            return Format::R32G32_UINT;
        case VK_FORMAT_R32_SFLOAT:
            return Format::R32_SFLOAT;
        case VK_FORMAT_R32_SINT:
            return Format::R32_SINT;
        case VK_FORMAT_R32_UINT:
            return Format::R32_UINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return Format::R16G16B16A16_SFLOAT;
        case VK_FORMAT_R16G16B16A16_SINT:
            return Format::R16G16B16A16_SINT;
        case VK_FORMAT_R16G16B16A16_UINT:
            return Format::R16G16B16A16_UINT;
        case VK_FORMAT_R16G16B16A16_SNORM:
            return Format::R16G16B16A16_SNORM;
        case VK_FORMAT_R16G16B16A16_UNORM:
            return Format::R16G16B16A16_UNORM;
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return Format::R16G16B16A16_SSCALED;
        case VK_FORMAT_R16G16B16A16_USCALED:
            return Format::R16G16B16A16_USCALED;
        case VK_FORMAT_R16G16B16_SFLOAT:
            return Format::R16G16B16_SFLOAT;
        case VK_FORMAT_R16G16B16_SINT:
            return Format::R16G16B16_SINT;
        case VK_FORMAT_R16G16B16_UINT:
            return Format::R16G16B16_UINT;
        case VK_FORMAT_R16G16B16_SNORM:
            return Format::R16G16B16_SNORM;
        case VK_FORMAT_R16G16B16_UNORM:
            return Format::R16G16B16_UNORM;
        case VK_FORMAT_R16G16B16_SSCALED:
            return Format::R16G16B16_SSCALED;
        case VK_FORMAT_R16G16B16_USCALED:
            return Format::R16G16B16_USCALED;
        case VK_FORMAT_R16G16_SFLOAT:
            return Format::R16G16_SFLOAT;
        case VK_FORMAT_R16G16_SINT:
            return Format::R16G16_SINT;
        case VK_FORMAT_R16G16_UINT:
            return Format::R16G16_UINT;
        case VK_FORMAT_R16G16_SNORM:
            return Format::R16G16_SNORM;
        case VK_FORMAT_R16G16_UNORM:
            return Format::R16G16_UNORM;
        case VK_FORMAT_R16G16_SSCALED:
            return Format::R16G16_SSCALED;
        case VK_FORMAT_R16G16_USCALED:
            return Format::R16G16_USCALED;
        case VK_FORMAT_R16_SFLOAT:
            return Format::R16_SFLOAT;
        case VK_FORMAT_R16_SINT:
            return Format::R16_SINT;
        case VK_FORMAT_R16_UINT:
            return Format::R16_UINT;
        case VK_FORMAT_R16_SNORM:
            return Format::R16_SNORM;
        case VK_FORMAT_R16_UNORM:
            return Format::R16_UNORM;
        case VK_FORMAT_R16_SSCALED:
            return Format::R16_SSCALED;
        case VK_FORMAT_R16_USCALED:
            return Format::R16_USCALED;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return Format::R8G8B8A8_SRGB;
        case VK_FORMAT_R8G8B8A8_SINT:
            return Format::R8G8B8A8_SINT;
        case VK_FORMAT_R8G8B8A8_UINT:
            return Format::R8G8B8A8_UINT;
        case VK_FORMAT_R8G8B8A8_SNORM:
            return Format::R8G8B8A8_SNORM;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return Format::R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return Format::R8G8B8A8_SSCALED;
        case VK_FORMAT_R8G8B8A8_USCALED:
            return Format::R8G8B8A8_USCALED;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return Format::B8G8R8A8_SRGB;
        case VK_FORMAT_B8G8R8A8_SINT:
            return Format::B8G8R8A8_SINT;
        case VK_FORMAT_B8G8R8A8_UINT:
            return Format::B8G8R8A8_UINT;
        case VK_FORMAT_B8G8R8A8_SNORM:
            return Format::B8G8R8A8_SNORM;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return Format::B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SSCALED:
            return Format::B8G8R8A8_SSCALED;
        case VK_FORMAT_B8G8R8A8_USCALED:
            return Format::B8G8R8A8_USCALED;
        case VK_FORMAT_R8G8B8_SRGB:
            return Format::R8G8B8_SRGB;
        case VK_FORMAT_R8G8B8_SINT:
            return Format::R8G8B8_SINT;
        case VK_FORMAT_R8G8B8_UINT:
            return Format::R8G8B8_UINT;
        case VK_FORMAT_R8G8B8_SNORM:
            return Format::R8G8B8_SNORM;
        case VK_FORMAT_R8G8B8_UNORM:
            return Format::R8G8B8_UNORM;
        case VK_FORMAT_R8G8B8_SSCALED:
            return Format::R8G8B8_SSCALED;
        case VK_FORMAT_R8G8B8_USCALED:
            return Format::R8G8B8_USCALED;
        case VK_FORMAT_B8G8R8_SRGB:
            return Format::B8G8R8_SRGB;
        case VK_FORMAT_B8G8R8_SINT:
            return Format::B8G8R8_SINT;
        case VK_FORMAT_B8G8R8_UINT:
            return Format::B8G8R8_UINT;
        case VK_FORMAT_B8G8R8_SNORM:
            return Format::B8G8R8_SNORM;
        case VK_FORMAT_B8G8R8_UNORM:
            return Format::B8G8R8_UNORM;
        case VK_FORMAT_B8G8R8_SSCALED:
            return Format::B8G8R8_SSCALED;
        case VK_FORMAT_B8G8R8_USCALED:
            return Format::B8G8R8_USCALED;
        case VK_FORMAT_R8G8_SRGB:
            return Format::R8G8_SRGB;
        case VK_FORMAT_R8G8_SINT:
            return Format::R8G8_SINT;
        case VK_FORMAT_R8G8_UINT:
            return Format::R8G8_UINT;
        case VK_FORMAT_R8G8_SNORM:
            return Format::R8G8_SNORM;
        case VK_FORMAT_R8G8_UNORM:
            return Format::R8G8_UNORM;
        case VK_FORMAT_R8G8_SSCALED:
            return Format::R8G8_SSCALED;
        case VK_FORMAT_R8G8_USCALED:
            return Format::R8G8_USCALED;
        case VK_FORMAT_R8_SRGB:
            return Format::R8_SRGB;
        case VK_FORMAT_R8_SINT:
            return Format::R8_SINT;
        case VK_FORMAT_R8_UINT:
            return Format::R8_UINT;
        case VK_FORMAT_R8_SNORM:
            return Format::R8_SNORM;
        case VK_FORMAT_R8_UNORM:
            return Format::R8_UNORM;
        case VK_FORMAT_R8_SSCALED:
            return Format::R8_SSCALED;
        case VK_FORMAT_R8_USCALED:
            return Format::R8_USCALED;
        case VK_FORMAT_S8_UINT:
            return Format::S8_UINT;
        case VK_FORMAT_D32_SFLOAT:
            return Format::D32_SFLOAT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return Format::D32_UNORM_S8_UINT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return Format::D24_UNORM_S8_UINT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return Format::D16_UNORM_S8_UINT;
        default:
            SERROR("Trying to convert a non-valid format.");
            return Format::UNDEFINED;
    }
}

constexpr VkFormat ConvertVertexFormat(VertexFormat InFormat)
{
    switch (InFormat)
    {
        default:
        case VertexFormat::FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::FLOAT2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::FLOAT3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::FLOAT4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexFormat::MAT4:
            return VK_FORMAT_R32G32B32A32_SFLOAT; // TODO MAT4
        case VertexFormat::BYTE:
            return VK_FORMAT_R8_SINT;
        case VertexFormat::BYTE4N:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case VertexFormat::UBYTE:
            return VK_FORMAT_R8_UINT;
        case VertexFormat::UBYTE4N:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VertexFormat::SHORT2:
            return VK_FORMAT_R16G16_SINT;
        case VertexFormat::SHORT2N:
            return VK_FORMAT_R16G16_SNORM;
        case VertexFormat::SHORT4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VertexFormat::SHORT4N:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case VertexFormat::UINT:
            return VK_FORMAT_R32_UINT;
        case VertexFormat::UINT2:
            return VK_FORMAT_R32G32_UINT;
        case VertexFormat::UINT4:
            return VK_FORMAT_R32G32B32A32_UINT;
    }
}

constexpr VkCompareOp ConvertCompareOperation(CompareOperation operation)
{
    switch (operation)
    {
        default:
        case CompareOperation::NEVER:
            return VK_COMPARE_OP_NEVER;
        case CompareOperation::LESS:
            return VK_COMPARE_OP_LESS;
        case CompareOperation::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case CompareOperation::LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperation::GREATER:
            return VK_COMPARE_OP_GREATER;
        case CompareOperation::NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperation::GREATER_OR_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperation::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
    }
}

constexpr inline VkAttachmentLoadOp ConvertLoadOperation(Attachment::LoadOp InOperation)
{
    switch (InOperation)
    {
        case Attachment::LoadOp::LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case Attachment::LoadOp::CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case Attachment::LoadOp::DONTCARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
    }
}

constexpr inline VkAttachmentStoreOp ConvertStoreOperation(Attachment::StoreOp InOperation)
{
    switch (InOperation)
    {
        case Attachment::StoreOp::STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case Attachment::StoreOp::DONTCARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            return VK_ATTACHMENT_STORE_OP_NONE;
    }
}

constexpr VkShaderStageFlagBits ConvertShaderStage(ShaderStageType stage)
{
    switch (stage)
    {
        case ShaderStageType::FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStageType::VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStageType::UNDEFINED:
        default:
            return VK_SHADER_STAGE_ALL;
    }
}

constexpr VkDescriptorType ConvertDescriptorType(UniformType InType)
{
    switch (InType)
    {
        default:
        case UniformType::UNIFORM:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case UniformType::SAMPLED:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
}

constexpr VkImageLayout ConvertImageLayout(BindPoint InLayout)
{
    switch (InLayout)
    {
        case BindPoint::RENDER_TARGET:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case BindPoint::DEPTH_STENCIL:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case BindPoint::SHADER_SAMPLE:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case BindPoint::NONE:
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
