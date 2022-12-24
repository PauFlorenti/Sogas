#pragma once

#include "render/render_device.h"
#include <vulkan/vulkan.h>
#include "vulkan_vertex_declaration.h"

namespace Sogas {
namespace Vk {
    struct VulkanVertex
    {
        glm::vec3 position;
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

} // Vk
} // Sogas
