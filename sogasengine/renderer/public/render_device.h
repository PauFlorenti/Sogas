#pragma once

#include "buffer.h"
#include "commandbuffer.h"
#include "device_resource_pool.h"
#include "device_resources.h"
#include "render_types.h"
#include "renderpass.h"
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

struct DeviceDescriptor
{
    Memory::Allocator* allocator;
    void*              window = nullptr;
    u16                width  = 0;
    u16                height = 0;

    DeviceDescriptor& SetWindow(void* InWindow, u16 InWidth, u16 InHeight);
    DeviceDescriptor& SetAllocator(Memory::Allocator* InAllocator);
};

class GPU_device
{
  public:
    static std::shared_ptr<GPU_device> create(GraphicsAPI api, std::vector<const char*> extensions);

    virtual ~GPU_device(){};

    virtual GraphicsAPI getApiType() const = 0;

    // TODO create general capabilities struct
    // virtual capabilities getCapabilities() const = 0;
    // TODO create general statistics struct
    // virtual statistics getStatistics() const = 0;

    // clang-format off

    virtual bool                     Init(const DeviceDescriptor& InDescriptor) = 0;
    virtual void                     shutdown() = 0;

    // Create gpu resources
    virtual BufferHandle              CreateBuffer(const BufferDescriptor& InDescriptor) = 0;
    virtual TextureHandle             CreateTexture(const TextureDescriptor& InDescriptor) = 0;
    virtual ShaderStateHandle         CreateShaderState(const ShaderStateDescriptor& InDescriptor) = 0;
    virtual SamplerHandle             CreateSampler(const SamplerDescriptor& InDescriptor) = 0;
    virtual DescriptorSetHandle       CreateDescriptorSet(const DescriptorSetDescriptor& InDescriptor) = 0;
    virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDescriptor& InDescriptor) = 0;
    virtual PipelineHandle            CreatePipeline(const PipelineDescriptor& InDescriptor) = 0;
    virtual RenderPassHandle          CreateRenderPass(const RenderPassDescriptor& InDescriptor) = 0;

    virtual void DestroyBuffer(BufferHandle InHandle) = 0;
    virtual void DestroyTexture(TextureHandle InHandle) = 0;
    virtual void DestroyShaderState(ShaderStateHandle InHandle) = 0;
    virtual void DestroySampler(SamplerHandle InHandle) = 0;
    virtual void DestroyDescriptorSet(DescriptorSetHandle InHandle) = 0;
    virtual void DestroyDescriptorSetLayout(DescriptorSetLayoutHandle InHandle) = 0;
    virtual void DestroyPipeline(PipelineHandle InPipelineHandle) = 0;
    virtual void DestroyRenderPass(RenderPassHandle InHandle) = 0;

    virtual std::vector<i8> ReadShaderBinary(std::string InFilename) = 0;
    virtual CommandBuffer* GetCommandBuffer(bool begin) = 0;
    virtual void QueueCommandBuffer(CommandBuffer* cmd) = 0;

    virtual void BeginFrame() = 0;
    virtual void Present() = 0;

    virtual void                     CreateSwapchain(GLFWwindow* window) = 0;
    virtual void                     CreateTexture(Texture *texture, void* data) const = 0;
    virtual std::shared_ptr<Texture> CreateTexture(TextureDescriptor desc, void* data = nullptr) const = 0;

    // API calls
    // This are commands that will execute when submitCommands is called.
    //virtual void SetWindowSize(std::shared_ptr<Swapchain> InSwapchain, const u32& width, const u32& height) = 0;
    virtual void SetTopology(PrimitiveTopology topology) = 0;

    // clang-format on

    virtual RenderPassHandle        GetSwapchainRenderpass()   = 0;
    virtual const RenderPassOutput& GetSwapchainOutput() const = 0;

    Memory::Allocator* allocator = nullptr;

    ResourcePool buffers;
    ResourcePool textures;
    ResourcePool shaders;
    ResourcePool samplers;
    ResourcePool descriptorSets;
    ResourcePool descriptorSetLayouts;
    ResourcePool pipelines;
    ResourcePool renderpasses;

    RenderPassHandle swapchain_renderpass;
    SamplerHandle    default_sampler;

    std::vector<CommandBuffer*> queued_command_buffers;

    TextureHandle depth_texture;

    void* window = nullptr;

  protected:
    GraphicsAPI api_type;
};

} // namespace Renderer
} // namespace Sogas
