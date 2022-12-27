#pragma once

#include "render_types.h"

// !TEMP should be removed
#include "components/camera_component.h"

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
        virtual void BeginRenderPass(const Swapchain* swapchain, CommandBuffer cmd) = 0;
        virtual void EndRenderPass(CommandBuffer cmd) = 0;
        virtual void CreateSwapchain(const SwapchainDescriptor& desc, Swapchain* swapchain) = 0;
        virtual void CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const = 0;
        virtual void CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const = 0;
        virtual void CreateRenderPass(const RenderPassDescriptor* desc, RenderPass* renderpass) const = 0;
        virtual void CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass = nullptr) const = 0;
        virtual void CreateAttachment() const = 0;
        virtual void CreateShader(ShaderStage stage, const char* filename, Shader* shader) const = 0;

        // API calls
        // This are commands that will execute when submitCommands is called.
        virtual void BindVertexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) = 0;
        virtual void BindIndexBuffer(const GPUBuffer* buffer, CommandBuffer cmd) = 0;
        virtual void BindPipeline(const Pipeline* pipeline, CommandBuffer cmd) = 0;
        virtual void SetTopology(PrimitiveTopology topology) = 0;
        virtual void Draw(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
        virtual void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
        virtual void ActivateObject(const glm::mat4& model, const glm::vec4& color, CommandBuffer cmd) = 0;
        virtual void activateCamera(const TCompCamera* camera) = 0;
        
    protected:
        GraphicsAPI api_type;
    };
} // Sogas
