#pragma once

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

    struct VulkanBuffer
    {
        VkBuffer buffer         = VK_NULL_HANDLE;
        VkDeviceMemory memory   = VK_NULL_HANDLE;
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

} // Vk
} // Sogas
