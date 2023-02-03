#pragma once

#include "vulkan_types.h"
#include "render/vulkan/vulkan_commandbuffer.h"

namespace Sogas {
namespace Vk
{

    class VulkanDevice : public GPU_device
    {
        friend class VulkanAttachment;
        friend class VulkanBuffer;
        friend class VulkanDescriptorSet;
        friend class VulkanPipeline;
        friend class VulkanRenderPass;
        friend class VulkanShader;
        friend class VulkanTexture;
    public:
        explicit VulkanDevice(GraphicsAPI apiType, void* device);
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&&) = delete;
        const VulkanDevice& operator=(const VulkanDevice& other) = delete;
        ~VulkanDevice() override;

        GraphicsAPI getApiType() const { return api_type; }

        constexpr u32 GetFrameCount() const { return FrameCount; }
        constexpr u32 GetFrameIndex() const { return GetFrameCount() % MAX_FRAMES_IN_FLIGHT; } 

        bool Init() override;
        void shutdown() override;
        CommandBuffer BeginCommandBuffer() override;
        void SubmitCommandBuffers() override;
        void BeginRenderPass(std::shared_ptr<Swapchain> swapchain, CommandBuffer cmd) override;
        void BeginRenderPass(RenderPass* InRenderpass, CommandBuffer cmd) override;
        void EndRenderPass(CommandBuffer cmd) override;
        void CreateSwapchain(const SwapchainDescriptor& desc, std::shared_ptr<Swapchain> swapchain) override;
        void CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const override;
        std::unique_ptr<Renderer::Buffer> CreateBuffer(Renderer::BufferDescriptor desc, void* data) const override;
        void CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const override;
        void CreateRenderPass(const RenderPassDescriptor* desc, RenderPass* renderpass) const override;
        void CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass = nullptr) const override;
        void CreateAttachment() const override {};
        void CreateShader(ShaderStage stage, const char* filename, Shader* shader) const override;
        void UpdateDescriptorSet(const Pipeline* InPipeline) const override;
        void CreateAttachment(AttachmentFramebuffer* InAttachment) const override;

        // API calls ...
        void BindVertexBuffer(const std::unique_ptr<Renderer::Buffer>& buffer, CommandBuffer cmd) override;
        void BindIndexBuffer(const std::unique_ptr<Renderer::Buffer>& buffer, CommandBuffer cmd) override;
        void BindPipeline(const Pipeline* InPipeline, CommandBuffer& cmd) override;
        void BindDescriptor(CommandBuffer cmd) override;
        void BindBuffer(const std::unique_ptr<Renderer::Buffer>& InBuffer, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet, const u32 InOffset = 0) override;
        void BindTexture(const Texture* InTexture, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) override;
        void BindAttachment(const AttachmentFramebuffer* InAttachment, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) override;
        void SetTopology(PrimitiveTopology topology) override;
        void Draw(const u32 count, const u32 offset, CommandBuffer cmd) override;
        void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) override;
        void PushConstants(const void* InData, const u32 InSize, CommandBuffer cmd) override;
        void UpdateBuffer(const std::unique_ptr<Renderer::Buffer>& InBuffer, const void* InData, const u32 InDataSize, const u32 InOffset, CommandBuffer cmd) override;
        void WaitCommand(CommandBuffer& cmd, CommandBuffer& cmdToWait) override;
        //void WaitCommand(CommandBuffer& cmd, Swapchain* swapchainToWait) override;

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

        VkSemaphore beginSemaphore  = VK_NULL_HANDLE;
        VkSemaphore endSemaphore    = VK_NULL_HANDLE;

        std::vector<std::unique_ptr<VulkanCommandBuffer>> commandBuffers;
        u32 commandBufferCounter{0};

        CHandle MainCamera;

        bool CreateInstance();
        void SetupDebugMessenger();
        void PickPhysicalDevice();
        bool CreateDevice();
        bool CheckValidationLayersSupport();
        void CreateCommandResources();
        u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const;
    };

} // Vk
} // Soogas
