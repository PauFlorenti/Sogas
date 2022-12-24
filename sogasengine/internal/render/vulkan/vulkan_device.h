#pragma once

#include "vulkan_types.h"
// ! TEMP should not communicate directly with both of these
#include "resources/mesh.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Sogas {
namespace Vk
{
    class VulkanDevice : public GPU_device
    {
        friend class VulkanBuffer;
        friend class VulkanTexture;
    public:
        VulkanDevice(GraphicsAPI apiType, void* device);
        ~VulkanDevice() override;

        GraphicsAPI getApiType() const { return api_type; }

        bool Init() override;
        bool beginFrame() override;
        void submitRenderCommands() override {};
        void endFrame() override;
        void shutdown() override;
        void CreateSwapchain(const SwapchainDescriptor& desc, Swapchain* swapchain) override;
        void CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const override;
        void CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const override;
        //void CreateRenderPass() override {};
        void CreatePipeline() override {};
        void CreateAttachment() override {};
        void CreateShader() override {};

        // API calls ...
        void BindVertexBuffer(const GPUBuffer* buffer) override;
        void BindIndexBuffer(const GPUBuffer* buffer) override;
        void SetTopology(PrimitiveTopology topology) override;
        void Draw(const u32 count, const u32 offset) override;
        void DrawIndexed(const u32 count, const u32 offset) override;
        void activateObject(const glm::mat4& model, const glm::vec4& color) override;
        void activateCamera(const TCompCamera* camera) override;
        void CreateTexture(){};
        void DestroyTexture(){};

    private:
        VkInstance       Instance   = VK_NULL_HANDLE;
        VkDevice         Handle     = VK_NULL_HANDLE;
        VkPhysicalDevice Gpu        = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        std::vector<u32> queueFamilies;
        u32     GraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        u32     PresentFamily  = VK_QUEUE_FAMILY_IGNORED;
        u32     TransferFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue GraphicsQueue  = VK_NULL_HANDLE;
        VkQueue PresentQueue   = VK_NULL_HANDLE;
        VkQueue TransferQueue  = VK_NULL_HANDLE;

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

        // Remove
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
        void CreateUniformBuffer();
        void CreateDescriptorPools();
        void CreateDescriptorSets();
        void UpdateUniformBuffer();
        void Submit();
        u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const;

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
