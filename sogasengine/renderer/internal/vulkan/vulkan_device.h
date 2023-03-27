#pragma once

#include "device_resources.h"
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
    friend class VulkanCommandBufferResources;
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

    // Init/shutownd
    bool Init(const DeviceDescriptor& InDescriptor) override;
    void shutdown() override;

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

    void                      DestroyBuffer(BufferHandle InHandle) override;
    void                      DestroyTexture(TextureHandle InHandle) override;
    void                      DestroyShaderState(ShaderStateHandle InHandle) override;
    void                      DestroySampler(SamplerHandle InHandle) override;
    void                      DestroyDescriptorSet(DescriptorSetHandle InHandle) override;
    void                      DestroyDescriptorSetLayout(DescriptorSetLayoutHandle InHandle) override;
    void                      DestroyPipeline(PipelineHandle InPipelineHandle) override;
    void                      DestroyRenderPass(RenderPassHandle InHandle) override;

    // clang-format on
    void BeginFrame() override;
    void Present() override;

    std::vector<i8> ReadShaderBinary(std::string InFilename) override;
    CommandBuffer*  GetCommandBuffer(bool begin) override;
    CommandBuffer*  GetInstantCommandBuffer() override;
    void            QueueCommandBuffer(CommandBuffer* cmd) override;

    GraphicsAPI             getApiType() const;
    const VkPhysicalDevice& GetGPU() const;
    const u32               GetFamilyQueueIndex();
    const u32               GetFrameCount() const;
    const u32               GetFrameIndex() const;
    RenderPassHandle        GetSwapchainRenderpass() override;
    const RenderPassOutput& GetSwapchainOutput() const override;
    VkRenderPass            GetVulkanRenderPass(const RenderPassOutput& InOutput, std::string InName);
    const VkQueue           GetGraphicsQueue();
    VulkanSampler*          GetDefaultSampler();

  private:
    bool CreateInstance();
    void PickPhysicalDevice();
    bool CreateDevice();
    void SetupDebugMessenger();
    bool CheckValidationLayersSupport();

    u32  FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const;
    void CreateSwapchain(GLFWwindow* window) override;

    VulkanBuffer*              GetBufferResource(BufferHandle handle);
    VulkanShaderState*         GetShaderResource(ShaderStateHandle handle);
    VulkanSampler*             GetSamplerResource(SamplerHandle handle);
    VulkanTexture*             GetTextureResource(TextureHandle handle);
    VulkanDescriptorSet*       GetDescriptorSetResource(DescriptorSetHandle handle);
    VulkanDescriptorSetLayout* GetDescriptorSetLayoutResource(DescriptorSetLayoutHandle handle);
    VulkanPipeline*            GetPipelineResource(PipelineHandle handle);
    VulkanRenderPass*          GetRenderPassResource(RenderPassHandle handle);

    void DestroyPipelineInstant(ResourceHandle InHandle);

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

    VkFence fence[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore beginSemaphore = VK_NULL_HANDLE;
    VkSemaphore endSemaphore   = VK_NULL_HANDLE;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
