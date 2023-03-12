#pragma once

#include "buffer.h"
#include "device_resource_pool.h"
#include "device_resources.h"
#include "render_types.h"
#include "renderpass.h"
#include "swapchain.h"
#include "texture.h"

struct GLFWwindow;

namespace Sogas
{
namespace Memory
{
struct Allocator;
}
namespace Renderer
{
class GPU_device
{
  public:
    static std::shared_ptr<GPU_device> create(GraphicsAPI api, void* device, std::vector<const char*> extensions, Memory::Allocator* InAllocator);

    virtual ~GPU_device(){};

    virtual GraphicsAPI getApiType() const = 0;

    // TODO create general capabilities struct
    // virtual capabilities getCapabilities() const = 0;
    // TODO create general statistics struct
    // virtual statistics getStatistics() const = 0;

    // clang-format off

    virtual bool                     Init() = 0;
    virtual void                     shutdown() = 0;

    virtual CommandBuffer            BeginCommandBuffer() = 0;
    virtual void                     SubmitCommandBuffers() = 0;
    virtual void                     BeginRenderPass(std::shared_ptr<Swapchain> swapchain, CommandBuffer cmd) = 0;
    virtual void                     BeginRenderPass(RenderPass* InRenderpass, CommandBuffer cmd) = 0;
    virtual void                     EndRenderPass(CommandBuffer cmd) = 0;

    // Create gpu resources
    virtual void                     CreateSwapchain(std::shared_ptr<Swapchain> swapchain, GLFWwindow* window) = 0;
    virtual BufferHandle             CreateBuffer(BufferDescriptor& InDescriptor) = 0;
    virtual std::shared_ptr<Buffer>  CreateBuffer(BufferDescriptor desc, void* data) const = 0;
    virtual std::shared_ptr<Buffer>  CreateBuffer(const u32& size, const u64& element_size, BufferBindingPoint binding, BufferUsage usage, void* data) const = 0;
    virtual void                     CreateTexture(Texture *texture, void* data) const = 0;
    virtual std::shared_ptr<Texture> CreateTexture(TextureDescriptor desc, void* data = nullptr) const = 0;
    virtual void                     CreateRenderPass(RenderPass* renderpass) const = 0;
    virtual void                     CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass = nullptr) const = 0;
    virtual void                     CreateAttachment() const = 0;
    virtual void                     CreateShader(ShaderStage stage, std::string filename, Shader* shader) const = 0;
    virtual void                     UpdateDescriptorSet(const Pipeline* InPipeline) const = 0;
    virtual void                     CreateAttachment(AttachmentFramebuffer* InAttachment) const = 0;

    // API calls
    // This are commands that will execute when submitCommands is called.
    virtual void SetWindowSize(std::shared_ptr<Swapchain> InSwapchain, const u32& width, const u32& height) = 0;
    virtual void BindVertexBuffer(const std::shared_ptr<Buffer>& buffer, CommandBuffer cmd) = 0;
    virtual void BindIndexBuffer(const std::shared_ptr<Buffer>& buffer, CommandBuffer cmd) = 0;
    virtual void BindPipeline(const Pipeline* InPipeline, CommandBuffer& cmd) = 0;
    virtual void BindDescriptor(CommandBuffer cmd) = 0;
    virtual void BindBuffer(const std::shared_ptr<Buffer>& InBuffer, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet, const u32 InOffset = 0) = 0;
    virtual void BindTexture(const Texture*  InTexture, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) = 0;
    virtual void BindAttachment(const AttachmentFramebuffer* InAttachment, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) = 0;
    virtual void SetTopology(PrimitiveTopology topology) = 0;
    virtual void Draw(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
    virtual void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) = 0;
    virtual void PushConstants(const void* InData, const u32 Size, CommandBuffer cmd) = 0;
    virtual void UpdateBuffer(const std::shared_ptr<Buffer>& InBuffer, const void* InData, const u32 InDataSize, const u32 InOffset, CommandBuffer cmd) = 0;
    virtual void WaitCommand(CommandBuffer& cmd, CommandBuffer& cmdToWait) = 0;

    // clang-format on

    Memory::Allocator* allocator = nullptr;

    ResourcePool buffers;
    ResourcePool textures;

  protected:
    GraphicsAPI api_type;
};

} // namespace Renderer
} // namespace Sogas
