#pragma once

#include "vulkan_commandbuffer.h"
#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

struct VulkanShaderState;

// TODO Make SASSERT_MSG to receive parameters so we can pass the code that failed.
#define vkcheck(result)               \
    {                                 \
        SASSERT(result == VK_SUCCESS) \
    }
class VulkanDevice : public GPU_device
{
    friend class VulkanAttachment;
    friend class VulkanBuffer;
    friend class VulkanCommandBuffer;
    friend class VulkanDescriptorSet;
    friend class VulkanDescriptorSetLayout;
    friend class VulkanPipeline;
    friend class VulkanRenderPass;
    friend class VulkanSampler;
    friend class VulkanShader;
    friend class VulkanSwapchain;
    friend class VulkanTexture;

  public:
    explicit VulkanDevice(GraphicsAPI              apiType,
                          std::vector<const char*> extensions);
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&)      = delete;
    ~VulkanDevice() override;

    const VulkanDevice& operator=(const VulkanDevice& other) = delete;

    // Helpers
    GraphicsAPI getApiType() const
    {
        return api_type;
    }
    const VkPhysicalDevice& GetGPU() const
    {
        return Physical_device;
    }
    constexpr u32 GetFrameCount() const
    {
        return FrameCount;
    }
    constexpr u32 GetFrameIndex() const
    {
        return GetFrameCount() % MAX_FRAMES_IN_FLIGHT;
    }

    // Init/shutownd
    bool Init(const DeviceDescriptor& InDescriptor) override;
    void shutdown() override;

    CommandBuffer BeginCommandBuffer() override;
    void          SubmitCommandBuffers() override;
    //void BeginRenderPass(std::shared_ptr<Renderer::Swapchain> swapchain, CommandBuffer cmd) override {};
    void BeginRenderPass(Renderer::RenderPass* InRenderpass, CommandBuffer cmd) override;
    void EndRenderPass(CommandBuffer cmd) override;

    // clang-format off
    // Create gpu resources
    BufferHandle              CreateBuffer(const BufferDescriptor& InDescriptor) override;
    TextureHandle             CreateTexture(const TextureDescriptor& InDescriptor) override;
    ShaderStateHandle         CreateShaderState(const ShaderStateDescriptor& InDescriptor) override;
    SamplerHandle             CreateSampler(const SamplerDescriptor& InDescriptor) override;
    DescriptorSetHandle       CreateDescriptorSet(const DescriptorSetDescriptor& InDescriptor) override;
    DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDescriptor& InDescriptor) override;
    PipelineHandle            CreatePipeline(const PipelineDescriptor& InDescriptor) override;
    RenderPassHandle          CreateRenderPass(const RenderPassDescriptor& InDescriptor) override;

    void DestroyBuffer(BufferHandle InHandle) override;
    void DestroyTexture(TextureHandle InHandle) override;
    void DestroyShaderState(ShaderStateHandle InHandle) override;
    void DestroySampler(SamplerHandle InHandle) override;
    void DestroyDescriptorSet(DescriptorSetHandle InHandle) override;
    void DestroyDescriptorSetLayout(DescriptorSetLayoutHandle InHandle) override;
    void DestroyPipeline(PipelineHandle InPipInHandleeline) override;
    void DestroyRenderPass(RenderPassHandle InHandle) override;

    void                              CreateSwapchain(GLFWwindow *window) override;
    void                              CreateTexture(Texture *texture, void* data = nullptr) const override;
    std::shared_ptr<Texture>          CreateTexture(TextureDescriptor desc, void* data = nullptr) const override;
    void                              CreatePipeline(const PipelineDescriptor *desc, Pipeline *pipeline, RenderPass *renderpass = nullptr) override;
    void                              CreateAttachment() const override{};
    void                              CreateShader(ShaderStageType stage, std::string filename, Shader *shader) const override;
    void                              UpdateDescriptorSet(const Pipeline *InPipeline) const override;

    // API calls ...
    //void SetWindowSize(std::shared_ptr<Swapchain> InSwapchain, const u32 &width, const u32 &height) override;
    void BindPipeline(const Pipeline *InPipeline, CommandBuffer &cmd) override;
    void BindDescriptor(CommandBuffer cmd) override;
    void BindTexture(const Texture *InTexture, const Pipeline *InPipeline, const u32 InSlot, const u32 InDescriptorSet = 0) override;
    void SetTopology(PrimitiveTopology topology) override;
    void Draw(const u32 count, const u32 offset, CommandBuffer cmd) override;
    void DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd) override;
    void PushConstants(const void *InData, const u32 InSize, CommandBuffer cmd) override;
    void WaitCommand(CommandBuffer &cmd, CommandBuffer &cmdToWait) override;
    // clang-format on

    RenderPassHandle        GetSwapchainRenderpass() override;
    const RenderPassOutput& GetSwapchainOutput() const override;

    VkRenderPass GetVulkanRenderPass(const RenderPassOutput& InOutput, std::string InName);

    VulkanBuffer*              GetBufferResource(BufferHandle handle);
    VulkanShaderState*         GetShaderResource(ShaderStateHandle handle);
    VulkanSampler*             GetSamplerResource(SamplerHandle handle);
    VulkanTexture*             GetTextureResource(TextureHandle handle);
    VulkanDescriptorSet*       GetDescriptorSetResource(DescriptorSetHandle handle);
    VulkanDescriptorSetLayout* GetDescriptorSetLayoutResource(DescriptorSetLayoutHandle handle);
    VulkanPipeline*            GetPipelineResource(PipelineHandle handle);
    VulkanRenderPass*          GetRenderPassResource(RenderPassHandle handle);

    const VkQueue GetGraphicsQueue() const
    {
        return GraphicsQueue;
    }

    VulkanSampler* GetDefaultSampler()
    {
        return GetSamplerResource(default_sampler);
    };

  private:
    // Device
    VkInstance                 Instance        = VK_NULL_HANDLE;
    VkDevice                   Handle          = VK_NULL_HANDLE;
    VkPhysicalDevice           Physical_device = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT   DebugMessenger  = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties Physical_device_properties;
    std::vector<const char*>   glfwExtensions;

    std::shared_ptr<VulkanSwapchain> swapchain;

    VkDescriptorPool descriptor_pool;

    // Queues
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<u32>                     queueFamilies;
    u32                                  GraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
    u32                                  PresentFamily  = VK_QUEUE_FAMILY_IGNORED;
    u32                                  TransferFamily = VK_QUEUE_FAMILY_IGNORED;
    VkQueue                              GraphicsQueue  = VK_NULL_HANDLE;
    VkQueue                              PresentQueue   = VK_NULL_HANDLE;
    VkQueue                              TransferQueue  = VK_NULL_HANDLE;
    u32                                  FrameCount     = 0; // Number of frames since the beginning of the application.

    VkCommandPool   resourcesCommandPool[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer resourcesCommandBuffer[MAX_FRAMES_IN_FLIGHT];
    VkFence         fence[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore beginSemaphore = VK_NULL_HANDLE;
    VkSemaphore endSemaphore   = VK_NULL_HANDLE;

    std::vector<std::unique_ptr<VulkanCommandBuffer>> commandBuffers;
    u32                                               commandBufferCounter{0};

    bool bIsBindlessSupported{false};

    bool CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    bool CreateDevice();
    bool CheckValidationLayersSupport();
    void CreateCommandResources();
    u32  FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
