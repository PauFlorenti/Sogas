#pragma once

#include "render/render_device.h"
#include <vulkan/vulkan.h>
#include "vulkan_vertex_declaration.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Sogas {
namespace Vk {
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
        u32 vertexCount = 0;
        u32 vertexSize  = 0;
        u32 vertexOffset= 0;
        u32 indexCount  = 0;
        u32 indexOffset = 0;

        u32 id = INVALID_ID;

        // TODO Make a buffer class
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    };

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
        switch(format)
        {
            case Format::R32G32B32A32_SFLOAT:
            case Format::R32G32B32A32_UINT:
            case Format::R32G32B32A32_SINT:
                return 16u;
            case Format::R32G32B32_SFLOAT:
            case Format::R32G32B32_UINT:
            case Format::R32G32B32_SINT:
                return 12u;
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
                return 4u;
            case Format::R8G8B8_SRGB:
            case Format::R8G8B8_SSCALED:
            case Format::R8G8B8_USCALED:
            case Format::R8G8B8_SNORM:
            case Format::R8G8B8_UNORM:
            case Format::R8G8B8_SINT:
            case Format::R8G8B8_UINT:
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
                break;
            case Format::R32G32B32A32_SINT:
                return VK_FORMAT_R32G32B32A32_SINT;
                break;
            case Format::R32G32B32A32_UINT:
                return VK_FORMAT_R32G32B32A32_SINT;
                break;
            case Format::R32G32B32_SFLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case Format::R32G32B32_SINT:
                return VK_FORMAT_R32G32B32_SINT;
                break;
            case Format::R32G32B32_UINT:
                return VK_FORMAT_R32G32B32_UINT;
                break;
            case Format::R32G32_SFLOAT:
                return VK_FORMAT_R32G32_SFLOAT;
                break;
            case Format::R32G32_SINT:
                return VK_FORMAT_R32G32_SINT;
                break;
            case Format::R32G32_UINT:
                return VK_FORMAT_R32G32_UINT;
                break;
            case Format::R32_SFLOAT:
                return VK_FORMAT_R32_SFLOAT;
                break;
            case Format::R32_SINT:
                return VK_FORMAT_R32_SFLOAT;
                break;
            case Format::R32_UINT:
                return VK_FORMAT_R32_UINT;
                break;
            case Format::R16G16B16A16_SFLOAT:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
                break;
            case Format::R16G16B16A16_SINT:
                return VK_FORMAT_R16G16B16A16_SINT;
                break;
            case Format::R16G16B16A16_UINT:
                return VK_FORMAT_R16G16B16A16_UINT;
                break;
            case Format::R16G16B16A16_SNORM:
                return VK_FORMAT_R16G16B16A16_SNORM;
                break;
            case Format::R16G16B16A16_UNORM:
                return VK_FORMAT_R16G16B16A16_UNORM;
                break;
            case Format::R16G16B16A16_SSCALED:
                return VK_FORMAT_R16G16B16A16_SSCALED;
                break;
            case Format::R16G16B16A16_USCALED:
                return VK_FORMAT_R16G16B16A16_USCALED;
                break;            
            case Format::R16G16B16_SFLOAT:
                return VK_FORMAT_R16G16B16_SFLOAT;
                break;
            case Format::R16G16B16_SINT:
                return VK_FORMAT_R16G16B16_SINT;
                break;
            case Format::R16G16B16_UINT:
                return VK_FORMAT_R16G16B16_UINT;
                break;
            case Format::R16G16B16_SNORM:
                return VK_FORMAT_R16G16B16_SNORM;
                break;
            case Format::R16G16B16_UNORM:
                return VK_FORMAT_R16G16B16_UNORM;
                break;
            case Format::R16G16B16_SSCALED:
                return VK_FORMAT_R16G16B16_SSCALED;
                break;
            case Format::R16G16B16_USCALED:
                return VK_FORMAT_R16G16B16_USCALED;
                break;
            case Format::R16G16_SFLOAT:
                return VK_FORMAT_R16G16_SFLOAT;
                break;
            case Format::R16G16_SINT:
                return VK_FORMAT_R16G16_SINT;
                break;
            case Format::R16G16_UINT:
                return VK_FORMAT_R16G16_UINT;
                break;
            case Format::R16G16_SNORM:
                return VK_FORMAT_R16G16_SNORM;
                break;
            case Format::R16G16_UNORM:
                return VK_FORMAT_R16G16_UNORM;
                break;
            case Format::R16G16_SSCALED:
                return VK_FORMAT_R16G16_SSCALED;
                break;
            case Format::R16G16_USCALED:
                return VK_FORMAT_R16G16_USCALED;
                break;
            case Format::R16_SFLOAT:
                return VK_FORMAT_R16_SFLOAT;
                break;
            case Format::R16_SINT:
                return VK_FORMAT_R16_SINT;
                break;
            case Format::R16_UINT:
                return VK_FORMAT_R16_UINT;
                break;
            case Format::R16_SNORM:
                return VK_FORMAT_R16_SNORM;
                break;
            case Format::R16_UNORM:
                return VK_FORMAT_R16_UNORM;
                break;
            case Format::R16_SSCALED:
                return VK_FORMAT_R16_SSCALED;
                break;
            case Format::R16_USCALED:
                return VK_FORMAT_R16_USCALED;
                break;
            case Format::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
                break;
            case Format::R8G8B8A8_SINT:
                return VK_FORMAT_R8G8B8A8_SINT;
                break;
            case Format::R8G8B8A8_UINT:
                return VK_FORMAT_R8G8B8A8_UINT;
                break;
            case Format::R8G8B8A8_SNORM:
                return VK_FORMAT_R8G8B8A8_SNORM;
                break;
            case Format::R8G8B8A8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;
                break;
            case Format::R8G8B8A8_SSCALED:
                return VK_FORMAT_R8G8B8A8_SSCALED;
                break;
            case Format::R8G8B8A8_USCALED:
                return VK_FORMAT_R8G8B8A8_USCALED;
                break;
            case Format::R8G8B8_SRGB:
                return VK_FORMAT_R8G8B8_SRGB;
                break;
            case Format::R8G8B8_SINT:
                return VK_FORMAT_R8G8B8_SINT;
                break;
            case Format::R8G8B8_UINT:
                return VK_FORMAT_R8G8B8_UINT;
                break;
            case Format::R8G8B8_SNORM:
                return VK_FORMAT_R8G8B8_SNORM;
                break;
            case Format::R8G8B8_UNORM:
                return VK_FORMAT_R8G8B8_UNORM;
                break;
            case Format::R8G8B8_SSCALED:
                return VK_FORMAT_R8G8B8_SSCALED;
                break;
            case Format::R8G8B8_USCALED:
                return VK_FORMAT_R8G8B8_USCALED;
                break;
            case Format::R8G8_SRGB:
                return VK_FORMAT_R8G8_SRGB;
                break;
            case Format::R8G8_SINT:
                return VK_FORMAT_R8G8_SINT;
                break;
            case Format::R8G8_UINT:
                return VK_FORMAT_R8G8_UINT;
                break;
            case Format::R8G8_SNORM:
                return VK_FORMAT_R8G8_SNORM;
                break;
            case Format::R8G8_UNORM:
                return VK_FORMAT_R8G8_UNORM;
                break;
            case Format::R8G8_SSCALED:
                return VK_FORMAT_R8G8_SSCALED;
                break;
            case Format::R8G8_USCALED:
                return VK_FORMAT_R8G8_USCALED;
                break;
            case Format::R8_SRGB:
                return VK_FORMAT_R8_SRGB;
                break;
            case Format::R8_SINT:
                return VK_FORMAT_R8_SINT;
                break;
            case Format::R8_UINT:
                return VK_FORMAT_R8_UINT;
                break;
            case Format::R8_SNORM:
                return VK_FORMAT_R8_SNORM;
                break;
            case Format::R8_UNORM:
                return VK_FORMAT_R8_UNORM;
                break;
            case Format::R8_SSCALED:
                return VK_FORMAT_R8_SSCALED;
                break;
            case Format::R8_USCALED:
                return VK_FORMAT_R8_USCALED;
                break;
            default:
                SERROR("Trying to convert a non-valid format.");
                return VK_FORMAT_UNDEFINED;
                break;
            }
        }

    constexpr VkShaderStageFlagBits ConvertShaderStage(ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::UNDEFINED:
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

} // Vk
} // Sogas
