#pragma once

#include "render/render_types.h"
#include "resources/mesh.h"
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

namespace Sogas {
namespace Vk
{
    class CRender
    {
    public:
        CRender();
        ~CRender(){};

        bool Init();
        void Update(f32 /*dt*/);
        bool PrepareFrame();
        void EndFrame();
        void DrawFrame();
        void Shutdown();

        // API calls ...
        bool CreateMesh(CMesh* mesh, std::vector<Vertex> vertices, PrimitiveTopology topology);
        bool CreateMesh(CMesh* mesh, std::vector<Vertex> vertices, std::vector<u32> indices, PrimitiveTopology topology);
        void Bind( const u32 renderId, PrimitiveTopology topology );
        void Draw(const u32 vertexCount, const u32 vertexOffset);
        void DrawIndexed(const u32 indexCount, const u32 indexOffset);
        void ActivateObject(const glm::mat4& model, const glm::vec4& color);
        void ActivateCamera();

    private:
        VkInstance Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

        VkPhysicalDevice Gpu = VK_NULL_HANDLE;

        u32 GraphicsFamily;
        VkQueue GraphicsQueue;
        u32 PresentFamily;
        VkQueue PresentQueue;

        VkDevice Device             = VK_NULL_HANDLE;
        VkSurfaceKHR Surface        = VK_NULL_HANDLE;
        VkSwapchainKHR Swapchain    = VK_NULL_HANDLE;
        VkSurfaceFormatKHR SurfaceFormat;
        VkPresentModeKHR PresentMode;
        VkExtent2D Extent;

        u32 FrameCount = 0; // Number of frames since the beginning of the application.
        u32 FrameIndex = 0; // The frame index in flight.
        u32 ImageIndex = 0;

        VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
        VkPipeline Pipeline             = VK_NULL_HANDLE;
        VkRenderPass RenderPass         = VK_NULL_HANDLE;

        VkCommandPool CommandPool       = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> CommandBuffers;
        std::vector<VkSemaphore> ImageAvailableSemaphore;
        std::vector<VkSemaphore> RenderFinishedSemaphore;
        std::vector<VkFence> InFlightFence;

        std::vector<VkImage> SwapchainImages;
        std::vector<VkImageView> SwapchainImageViews;
        std::vector<VkFramebuffer> SwapchainFramebuffers;

        std::vector<VkBuffer> UniformBuffers;
        std::vector<VkDeviceMemory> UniformBufferMemory;
        std::vector<void*> UniformBuffersMapped;

        VkDescriptorSetLayout DescriptorSetLayout   = VK_NULL_HANDLE;
        VkDescriptorPool DescriptorPool             = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> DescriptorSets;

        CHandle MainCamera;

        bool CreateInstance();
        void SetupDebugMessenger();
        void PickPhysicalDevice();
        bool CreateDevice();
        bool CheckValidationLayersSupport();
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void CreateSyncObjects();
        void CreateVertexBuffer();
        void CreateUniformBuffer();
        void CreateDescriptorPools();
        void CreateDescriptorSets();
        void UpdateUniformBuffer();
        void Submit();
        u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags);

        void CreateBuffer(
            VkDeviceSize size, 
            VkBufferUsageFlags usageFlags, 
            VkMemoryPropertyFlags memoryPropertyFlags, 
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory);

        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    };

} // Vk
} // Soogas
