#pragma once

#include "buffer.h"
#include "render_types.h"

// !TEMP should be removed
//#include "components/camera_component.h"

namespace Sogas 
{
    class GPU_device
    {
    public:

        static std::shared_ptr<GPU_device> create(GraphicsAPI api, void* device);

        virtual ~GPU_device() {};
        
        virtual GraphicsAPI getApiType() const = 0;

        // TODO create general capabilities struct
        //virtual capabilities getCapabilities() const = 0;
        // TODO create general statistics struct
        // virtual statistics getStatistics() const = 0;

        virtual bool Init() = 0;
        virtual void shutdown() = 0;
        virtual CommandBuffer BeginCommandBuffer() = 0;
        virtual void SubmitCommandBuffers() = 0;
        virtual void BeginRenderPass(std::shared_ptr<Swapchain> swapchain, CommandBuffer cmd) = 0;
        virtual void BeginRenderPass(RenderPass* InRenderpass, CommandBuffer cmd) = 0;
        virtual void EndRenderPass(CommandBuffer cmd) = 0;
        virtual void CreateSwapchain(const SwapchainDescriptor& desc, std::shared_ptr<Swapchain> swapchain) = 0;
        virtual void CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const = 0;
        virtual std::unique_ptr<Renderer::Buffer> CreateBuffer(Renderer::BufferDescriptor desc, void* data) const = 0;
        virtual void CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const = 0;
        virtual void CreateRenderPass(const RenderPassDescriptor* desc, RenderPass* renderpass) const = 0;
        virtual void CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass = nullptr) const = 0;
        virtual void CreateAttachment() const = 0;
        virtual void CreateShader(ShaderStage stage, const char* filename, Shader* shader) const = 0;
        virtual void UpdateDescriptorSet(const Pipeline* InPipeline) const = 0;
        virtual void CreateAttachment(AttachmentFramebuffer* InAttachment) const = 0;

        // API calls
        // This are commands that will execute when submitCommands is called.
        virtual void BindVertexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) = 0;
        virtual void BindVertexBuffer(const std::unique_ptr<Renderer::Buffer>& buffer, CommandBuffer cmd) = 0;
        virtual void BindIndexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) = 0;
        virtual void BindPipeline(const Pipeline* InPipeline, CommandBuffer& cmd) = 0;
        virtual void BindDescriptor(CommandBuffer cmd) = 0;
        virtual void BindBuffer(const GPUBuffer* InBuffer, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet, const u32 InOffset = 0) = 0;
        virtual void BindTexture(const Texture* InTexture, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) = 0;
        virtual void BindAttachment(const AttachmentFramebuffer* InAttachment, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) = 0;
        virtual void SetTopology(PrimitiveTopology topology) = 0;
        virtual void Draw(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
        virtual void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
        virtual void PushConstants(const void* InData, const u32 Size, CommandBuffer cmd) = 0;
        virtual void UpdateBuffer(const GPUBuffer* InBuffer, const void* InData, const u32 InDataSize, const u32 InOffset, CommandBuffer cmd) = 0;
        virtual void WaitCommand(CommandBuffer& cmd, CommandBuffer& cmdToWait) = 0;
        //virtual void WaitCommand(CommandBuffer& cmd, Swapchain* swapchainToWait) = 0;
        
    protected:
        GraphicsAPI api_type;
    };
} // Sogas
