#pragma once

#include "vulkan_types.h"
#include "render/vulkan/vulkan_commandbuffer.h"

namespace Sogas {
namespace Vk
{

    class VulkanDevice : public GPU_device
    {
        friend class VulkanBuffer;
        friend class VulkanPipeline;
        friend class VulkanShader;
        friend class VulkanTexture;
    public:
        VulkanDevice(GraphicsAPI apiType, void* device);
        ~VulkanDevice() override;

        GraphicsAPI getApiType() const { return api_type; }

        constexpr u32 GetFrameCount() const { return FrameCount; }
        constexpr u32 GetFrameIndex() const { return GetFrameCount() % MAX_FRAMES_IN_FLIGHT; } 

        bool Init() override;
        void shutdown() override;
        CommandBuffer BeginCommandBuffer() override;
        void SubmitCommandBuffers() override;
        void BeginRenderPass(const Swapchain* swapchain, CommandBuffer cmd) override;
        void EndRenderPass(CommandBuffer cmd) override;
        void CreateSwapchain(const SwapchainDescriptor& desc, Swapchain* swapchain) override;
        void CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const override;
        void CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const override;
        void CreateRenderPass(const RenderPassDescriptor* desc, RenderPass* renderpass) const override;
        void CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass = nullptr) const override;
        void CreateAttachment() const override {};
        void CreateShader(ShaderStage stage, const char* filename, Shader* shader) const override;

        // API calls ...
        void BindVertexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) override;
        void BindIndexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) override;
        void BindPipeline(const Pipeline* pipeline, CommandBuffer cmd) override;
        void SetTopology(PrimitiveTopology topology) override;
        void Draw(const u32 count, const u32 offset, CommandBuffer cmd) override;
        void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) override;
        void ActivateObject(const glm::mat4& model, const glm::vec4& color, CommandBuffer cmd) override;
        void activateCamera(const TCompCamera* camera) override;

    private:
        VkInstance       Instance   = VK_NULL_HANDLE;
        VkDevice         Handle     = VK_NULL_HANDLE;
        VkPhysicalDevice Gpu        = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

        // Queues
        std::vector<VkQueueFamilyProperties>    queueFamilyProperties;
        std::vector<u32>                        queueFamilies;
        u32     GraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        u32     PresentFamily  = VK_QUEUE_FAMILY_IGNORED;
        u32     TransferFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue GraphicsQueue  = VK_NULL_HANDLE;
        VkQueue PresentQueue   = VK_NULL_HANDLE;
        VkQueue TransferQueue  = VK_NULL_HANDLE;

        u32 FrameCount = 0; // Number of frames since the beginning of the application.

        VkCommandPool   resourcesCommandPool[MAX_FRAMES_IN_FLIGHT];
        VkCommandBuffer resourcesCommandBuffer[MAX_FRAMES_IN_FLIGHT];
        VkFence         fence[MAX_FRAMES_IN_FLIGHT];

        std::vector<std::unique_ptr<VulkanCommandBuffer>> commandBuffers;
        u32 commandBufferCounter{0};

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
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateCommandResources();
        void CreateUniformBuffer();
        void CreateDescriptorPools();
        void CreateDescriptorSets();
        void UpdateUniformBuffer();
        u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const;

        void CreateBuffer(
            VkDeviceSize size, 
            VkBufferUsageFlags usageFlags, 
            VkMemoryPropertyFlags memoryPropertyFlags, 
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory);

        VulkanCommandBuffer GetCommandBuffer(CommandBuffer& cmd) {
            SASSERT(cmd.IsValid());
            return *static_cast<VulkanCommandBuffer*>(cmd.internalState);
        }
    };

} // Vk
} // Soogas
