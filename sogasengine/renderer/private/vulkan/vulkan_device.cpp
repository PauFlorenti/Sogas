
#include "vulkan/vulkan_device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_commandbuffer.h"
#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_sampler.h"
#include "vulkan/vulkan_shader.h"
#include "vulkan/vulkan_swapchain.h"
#include "vulkan/vulkan_texture.h"
#include "vulkan/vulkan_types.h"

#include "public/sgs_memory.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

static std::unordered_map<u64, VkRenderPass> render_pass_cache;
static VulkanCommandBufferResources          commandbuffer_resources;

const std::vector<const char*> validationLayers         = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool validationLayersEnabled = false;
#else
const bool validationLayersEnabled = true;
#endif

static VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks*              pAllocator,
                                             VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                          VkDebugUtilsMessengerEXT     debugMessenger,
                                          const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* /*pUserData*/)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << "\n";
    }
    return VK_FALSE;
}

VulkanDevice::VulkanDevice(GraphicsAPI              apiType,
                           std::vector<const char*> extensions)
: glfwExtensions(std::move(extensions))
{
    api_type = apiType;
}

VulkanDevice::~VulkanDevice()
{
    shutdown();
}

bool VulkanDevice::Init(const DeviceDescriptor& InDescriptor)
{
    STRACE("Initializing Vulkan renderer ... ");

    allocator = InDescriptor.allocator;

    if (!CreateInstance())
    {
        STRACE("\tFailed to create Vulkan Instance!");
        return false;
    }

    if (!CreateDevice())
    {
        STRACE("\tFailed to create Vulkan Logical Device!");
        return false;
    }

    buffers.Init(allocator, 512, sizeof(VulkanBuffer));
    textures.Init(allocator, 512, sizeof(VulkanTexture));
    renderpasses.Init(allocator, 256, sizeof(VulkanRenderPass));
    pipelines.Init(allocator, 128, sizeof(VulkanPipeline));
    shaders.Init(allocator, 128, sizeof(VulkanShaderState));
    descriptorSets.Init(allocator, 128, sizeof(VulkanDescriptorSet));
    descriptorSetLayouts.Init(allocator, 128, sizeof(VulkanDescriptorSetLayout));
    samplers.Init(allocator, 32, sizeof(VulkanSampler));

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(Handle, &fenceInfo, nullptr, &fence[i]) != VK_SUCCESS)
        {
            SFATAL("Failed to create frame fence.");
            return false;
        }
    }

    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(InDescriptor.window);
    CreateSwapchain(window);

    commandbuffer_resources.init(this);

    static const u32 global_pool_elements = 128;

    // clang-format off
    std::vector<VkDescriptorPoolSize> pool_sizes =
      {
        {VK_DESCRIPTOR_TYPE_SAMPLER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, global_pool_elements}
    };
    // clang-format on

    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                    = global_pool_elements * pool_sizes.size();
    pool_info.poolSizeCount              = static_cast<u32>(pool_sizes.size());
    pool_info.pPoolSizes                 = pool_sizes.data();
    vkcheck(vkCreateDescriptorPool(Handle, &pool_info, nullptr, &descriptor_pool));

    SamplerDescriptor sampler_descriptor{};
    sampler_descriptor
      .SetAddressModeUVW(SamplerDescriptor::SamplerAddressMode::REPEAT, SamplerDescriptor::SamplerAddressMode::REPEAT, SamplerDescriptor::SamplerAddressMode::REPEAT)
      .SetMinMagMip(SamplerDescriptor::SamplerFilter::LINEAR, SamplerDescriptor::SamplerFilter::LINEAR, SamplerDescriptor::SamplerMipmapMode::LINEAR)
      .SetName("Default Sampler");
    default_sampler = CreateSampler(sampler_descriptor);

    TextureDescriptor depth_texture_descriptor = {nullptr, swapchain->width, swapchain->height, 1, 1, 0, Format::D32_SFLOAT, TextureDescriptor::TextureType::TEXTURE_TYPE_2D, "DepthTexture"};
    depth_texture                              = CreateTexture(depth_texture_descriptor);

    swapchain->output.SetDepth(Format::D32_SFLOAT);

    RenderPassDescriptor swapchain_renderpass_descriptor;
    swapchain_renderpass_descriptor
      .SetType(RenderPassType::SWAPCHAIN)
      .SetName("Swapchain")
      .SetOperations(RenderPassOperation::CLEAR, RenderPassOperation::CLEAR, RenderPassOperation::CLEAR);

    swapchain_renderpass = CreateRenderPass(swapchain_renderpass_descriptor);

    STRACE("Finished Initializing Vulkan device.\n");

    return true;
}

void VulkanDevice::shutdown()
{
    STRACE("Shutting down Vulkan renderer ...");

    vkDeviceWaitIdle(Handle);

    commandbuffer_resources.shutdown();

    STRACE("\tDestroying Command Pool ...");
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        //vkDestroyCommandPool(Handle, resourcesCommandPool[i], nullptr);
        vkDestroyFence(Handle, fence[i], nullptr);
    }

    vkDestroySemaphore(Handle, beginSemaphore, nullptr);
    vkDestroySemaphore(Handle, endSemaphore, nullptr);

    vkDestroyDescriptorPool(Handle, descriptor_pool, nullptr);

    DestroyTexture(depth_texture);
    DestroyRenderPass(swapchain_renderpass);
    DestroySampler(default_sampler);

    for (auto& resource : resource_deletion_queue)
    {
        switch (resource.type)
        {
            case ResourceType::BUFFER:
                DestroyBufferInstant(resource.handle);
                break;
            case ResourceType::TEXTURE:
                DestroyTextureInstant(resource.handle);
                break;
            case ResourceType::SHADER:
                DestroyShaderStateInstant(resource.handle);
                break;
            case ResourceType::SAMPLER:
                DestroySamplerInstant(resource.handle);
                break;
            case ResourceType::DESCRIPTOR_SET:
                DestroyDescriptorSetInstant(resource.handle);
                break;
            case ResourceType::DESCRIPTOR_SET_LAYOUT:
                DestroyDescriptorSetLayoutInstant(resource.handle);
                break;
            case ResourceType::PIPELINE:
                DestroyPipelineInstant(resource.handle);
                break;
            case ResourceType::RENDERPASS:
                DestroyRenderPassInstant(resource.handle);
                break;
        }
    }

    auto it = render_pass_cache.begin();
    while (it != render_pass_cache.end())
    {
        vkDestroyRenderPass(Handle, it->second, nullptr);
        ++it;
    }

    render_pass_cache.clear();

    // Swapchain pass not present in the cache
    VulkanRenderPass* swapchain_pass = GetRenderPassResource(swapchain_renderpass);
    vkDestroyRenderPass(Handle, swapchain_pass->renderpass, nullptr);

    swapchain->Destroy();

    samplers.Shutdown();
    descriptorSetLayouts.Shutdown();
    descriptorSets.Shutdown();
    shaders.Shutdown();
    pipelines.Shutdown();
    renderpasses.Shutdown();
    textures.Shutdown();
    buffers.Shutdown();

    STRACE("\tDestroying Vulkan logical device ...");
    vkDestroyDevice(Handle, nullptr);
    if (validationLayersEnabled)
    {
        DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    }
    STRACE("\tDestroying instance ...");
    vkDestroyInstance(Instance, nullptr);
    STRACE("\tInstance destroyed.");
    STRACE("Vulkan renderer has shut down.\n");
}

BufferHandle VulkanDevice::CreateBuffer(const BufferDescriptor& InDescriptor)
{
    return VulkanBuffer::Create(this, InDescriptor);
}

TextureHandle VulkanDevice::CreateTexture(const TextureDescriptor& InDescriptor)
{
    return VulkanTexture::Create(this, InDescriptor);
}

ShaderStateHandle VulkanDevice::CreateShaderState(const ShaderStateDescriptor& InDescriptor)
{
    return VulkanShader::Create(this, InDescriptor);
}

SamplerHandle VulkanDevice::CreateSampler(const SamplerDescriptor& InDescriptor)
{
    return VulkanSampler::Create(this, InDescriptor);
}
DescriptorSetHandle VulkanDevice::CreateDescriptorSet(const DescriptorSetDescriptor& InDescriptor)
{
    return VulkanDescriptorSet::Create(this, InDescriptor);
}

DescriptorSetLayoutHandle VulkanDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutDescriptor& InDescriptor)
{
    return VulkanDescriptorSet::Create(this, InDescriptor);
}

PipelineHandle VulkanDevice::CreatePipeline(const PipelineDescriptor& InDescriptor)
{
    return VulkanPipeline::Create(this, InDescriptor);
}

RenderPassHandle VulkanDevice::CreateRenderPass(const RenderPassDescriptor& InDescriptor)
{
    return VulkanRenderPass::Create(this, InDescriptor);
}

void VulkanDevice::DestroyBuffer(BufferHandle InHandle)
{
    if (InHandle.index < buffers.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::BUFFER, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid buffer.");
    }
}

void VulkanDevice::DestroyTexture(TextureHandle InHandle)
{
    if (InHandle.index < textures.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::TEXTURE, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid texture.");
    }
}
void VulkanDevice::DestroyShaderState(ShaderStateHandle InHandle)
{
    if (InHandle.index < shaders.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::SHADER, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid shader.");
    }
}

void VulkanDevice::DestroySampler(SamplerHandle InHandle)
{
    if (InHandle.index < samplers.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::SAMPLER, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid sampler.");
    }
}

void VulkanDevice::DestroyDescriptorSet(DescriptorSetHandle InHandle)
{
    if (InHandle.index < descriptorSets.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::DESCRIPTOR_SET, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid descriptor set.");
    }
}

void VulkanDevice::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle InHandle)
{
    if (InHandle.index < descriptorSetLayouts.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::DESCRIPTOR_SET_LAYOUT, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid descriptor set layout.");
    }
}

void VulkanDevice::DestroyPipeline(PipelineHandle InHandle)
{
    if (InHandle.index < pipelines.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::PIPELINE, InHandle.index, GetFrameCount()});
        auto pipeline = GetPipelineResource(InHandle);

        if (pipeline)
        {
            DestroyShaderState(pipeline->shader_state);
        }
    }
    else
    {
        SERROR("Trying to delete an invalid pipeline.");
    }
}

void VulkanDevice::DestroyRenderPass(RenderPassHandle InHandle)
{
    if (InHandle.index < renderpasses.pool_size)
    {
        resource_deletion_queue.push_back({ResourceType::RENDERPASS, InHandle.index, GetFrameCount()});
    }
    else
    {
        SERROR("Trying to delete an invalid renderpass.");
    }
}

void VulkanDevice::BeginFrame()
{
    const u32& frame_index = GetFrameIndex();
    if (vkGetFenceStatus(Handle, fence[frame_index]) != VK_SUCCESS)
    {
        vkWaitForFences(Handle, 1, &fence[frame_index], VK_TRUE, UINT64_MAX);
    }

    vkResetFences(Handle, 1, &fence[frame_index]);

    commandbuffer_resources.reset_pools(frame_index);
}

void VulkanDevice::Present()
{
    VkResult ok = vkAcquireNextImageKHR(Handle, swapchain->swapchain, UINT64_MAX, swapchain->presentCompleteSemaphore, VK_NULL_HANDLE, &swapchain->imageIndex);

    if (ok == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // TODO resize
        ++FrameCount;
        return;
    }

    std::vector<VkCommandBuffer> enqueued_command_buffers;
    enqueued_command_buffers.reserve(4);
    for (auto cmd : queued_command_buffers)
    {
        VulkanCommandBuffer* vulkan_cmd = static_cast<VulkanCommandBuffer*>(cmd);
        enqueued_command_buffers.push_back(vulkan_cmd->command_buffer);

        if (vulkan_cmd->is_recording && vulkan_cmd->current_renderpass && (vulkan_cmd->current_renderpass->type != RenderPassType::COMPUTE))
        {
            vkCmdEndRenderPass(vulkan_cmd->command_buffer);
        }

        vkEndCommandBuffer(vulkan_cmd->command_buffer);
    }

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit         = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount   = 1;
    submit.pWaitSemaphores      = &swapchain->presentCompleteSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores    = &swapchain->renderCompleteSemaphore;
    submit.commandBufferCount   = static_cast<u32>(enqueued_command_buffers.size());
    submit.pCommandBuffers      = enqueued_command_buffers.data();
    submit.pWaitDstStageMask    = wait_stages;

    vkQueueSubmit(GraphicsQueue, 1, &submit, fence[GetFrameIndex()]);

    VkPresentInfoKHR present_info   = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain->swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &swapchain->renderCompleteSemaphore;
    present_info.pImageIndices      = &swapchain->imageIndex;

    ok = vkQueuePresentKHR(GraphicsQueue, &present_info);

    queued_command_buffers.clear();

    if (ok == VK_ERROR_OUT_OF_DATE_KHR || ok == VK_SUBOPTIMAL_KHR || resized)
    {
        resized = false;

        // TODO resized swapchain.
        ++FrameCount;
        return;
    }

    ++FrameCount;

    // Resource deletion here ...
    if (!resource_deletion_queue.empty())
    {
        for (auto& resource : resource_deletion_queue)
        {
            if (resource.current_frame == GetFrameCount())
            {
                switch (resource.type)
                {
                    case ResourceType::BUFFER:
                        DestroyBufferInstant(resource.handle);
                        break;
                    case ResourceType::TEXTURE:
                        DestroyTextureInstant(resource.handle);
                        break;
                    case ResourceType::SHADER:
                        DestroyShaderStateInstant(resource.handle);
                        break;
                    case ResourceType::SAMPLER:
                        DestroySamplerInstant(resource.handle);
                        break;
                    case ResourceType::DESCRIPTOR_SET:
                        DestroyDescriptorSetInstant(resource.handle);
                        break;
                    case ResourceType::DESCRIPTOR_SET_LAYOUT:
                        DestroyDescriptorSetLayoutInstant(resource.handle);
                        break;
                    case ResourceType::PIPELINE:
                        DestroyPipelineInstant(resource.handle);
                        break;
                    case ResourceType::RENDERPASS:
                        DestroyRenderPassInstant(resource.handle);
                        break;
                }
            }
        }

        resource_deletion_queue.clear();
    }
}

std::vector<i8> VulkanDevice::ReadShaderBinary(std::string InFilename)
{
    return VulkanShader::ReadShaderFile(InFilename);
}

CommandBuffer* VulkanDevice::GetCommandBuffer(bool begin)
{
    return commandbuffer_resources.get_command_buffer(GetFrameIndex(), begin);
}

CommandBuffer* VulkanDevice::GetInstantCommandBuffer()
{
    return commandbuffer_resources.get_instant_command_buffer(GetFrameIndex());
}

void VulkanDevice::QueueCommandBuffer(CommandBuffer* cmd)
{
    queued_command_buffers.push_back(cmd);
}

GraphicsAPI VulkanDevice::getApiType() const
{
    return api_type;
}

const VkPhysicalDevice& VulkanDevice::GetGPU() const
{
    return Physical_device;
}

const u32 VulkanDevice::GetFamilyQueueIndex()
{
    return GraphicsFamily;
}

const u32 VulkanDevice::GetFrameCount() const
{
    return FrameCount;
}

const u32 VulkanDevice::GetFrameIndex() const
{
    return GetFrameCount() % MAX_FRAMES_IN_FLIGHT;
}

RenderPassHandle VulkanDevice::GetSwapchainRenderpass()
{
    return swapchain_renderpass;
}

const RenderPassOutput& VulkanDevice::GetSwapchainOutput() const
{
    return swapchain->output;
}

VkRenderPass VulkanDevice::GetVulkanRenderPass(const RenderPassOutput& InOutput, std::string InName)
{
    u64          hashed      = wyhash((void*)&InOutput, sizeof(RenderPassOutput), 0, _wyp);
    VkRenderPass render_pass = render_pass_cache.find(hashed) != render_pass_cache.end() ? render_pass_cache.at(hashed) : nullptr;
    if (render_pass)
    {
        return render_pass;
    }

    render_pass = VulkanRenderPass::CreateRenderPass(this, InOutput, InName);
    render_pass_cache.insert(std::pair<u64, VkRenderPass>(hashed, render_pass));

    return render_pass;
}

const VkQueue VulkanDevice::GetGraphicsQueue()
{
    return GraphicsQueue;
}

VulkanSampler* VulkanDevice::GetDefaultSampler()
{
    return GetSamplerResource(default_sampler);
};

// +---------+
// | PRIVATE |
// +---------+

bool VulkanDevice::CreateInstance()
{
    STRACE("\tCreating the Vulkan Instance ...");

    VkApplicationInfo applicationInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    applicationInfo.pApplicationName   = "Sogas app hardcoded";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName        = "Sogas Engine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.apiVersion         = VK_API_VERSION_1_3;

    // Checking for validation layer suppport
    if (validationLayersEnabled && !CheckValidationLayersSupport())
    {
        SERROR("\tRequired validation layers requested not found.");
    }

    // Enumerate all available extensions ...
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    STRACE("-- Available instance extensions --");
    for (VkExtensionProperties extension : availableExtensions)
    {
        STRACE("\t%s", extension.extensionName);
    }
    std::cout << "\t-- \t -- \t -- \n";

    SASSERT(glfwExtensions.empty() == false);
    if (validationLayersEnabled)
    {
        glfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Print the selected extensions ...

    VkInstanceCreateInfo instanceCreateInfo    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.pApplicationInfo        = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount   = static_cast<u32>(glfwExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions.data();
    instanceCreateInfo.enabledLayerCount       = validationLayersEnabled ? static_cast<u32>(validationLayers.size()) : 0;
    instanceCreateInfo.ppEnabledLayerNames     = validationLayersEnabled ? validationLayers.data() : nullptr;

    VkResult ok = vkCreateInstance(&instanceCreateInfo, nullptr, &Instance);

    if (ok != VK_SUCCESS || Instance == VK_NULL_HANDLE)
    {
        switch (ok)
        {
            case (VK_ERROR_OUT_OF_HOST_MEMORY):
            case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
            case (VK_ERROR_INITIALIZATION_FAILED):
            case (VK_ERROR_LAYER_NOT_PRESENT):
            case (VK_ERROR_EXTENSION_NOT_PRESENT):
            case (VK_ERROR_INCOMPATIBLE_DRIVER):
                SERROR("\tError creating the Vulkan Instance");
                break;
            default:
                break;
        }
        return false;
    }

    STRACE("\tSuccessfully created a Vulkan Instance.");

    SetupDebugMessenger();

    return true;
}

void VulkanDevice::PickPhysicalDevice()
{
    u32 PhysicalDeviceCount = 0;
    vkcheck(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));

    if (PhysicalDeviceCount == 0)
    {
        throw std::runtime_error("No available physical device!");
    }

    std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
    vkcheck(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));

    STRACE("Available Physical Devices ...");

    for (const auto gpu : PhysicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpu, &properties);
        STRACE("\tDevice Name: %s\tVendor ID: %i", properties.deviceName, properties.vendorID);
    }

    std::cout << "\t--\t--\t--\n\n";

    Physical_device = PhysicalDevices.at(0);
    vkGetPhysicalDeviceProperties(Physical_device, &Physical_device_properties);

    // TODO Make a proper Physical device selector ...
    // Is Device suitable ??
}

bool VulkanDevice::CreateDevice()
{
    PickPhysicalDevice();

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Physical_device, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Physical_device, &queueFamilyCount, queueFamilyProperties.data());

    u32 i = 0;
    for (const auto& queueFamily : queueFamilyProperties)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            GraphicsFamily = i;
            break;
        }
        ++i;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32>                        uniqueQueueFamilies = {GraphicsFamily};

    f32 queuePriority = 1.0f;
    for (u32 queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        createInfo.queueCount              = 1;
        createInfo.queueFamilyIndex        = queueFamily;
        createInfo.pQueuePriorities        = &queuePriority;
        queueCreateInfos.push_back(createInfo);
        queueFamilies.push_back(queueFamily);
    }

    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
      nullptr};

    VkPhysicalDeviceFeatures2 physical_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexing_features};
    vkGetPhysicalDeviceFeatures2(Physical_device, &physical_features2);

    bIsBindlessSupported =
      indexing_features.descriptorBindingPartiallyBound && indexing_features.runtimeDescriptorArray;

    VkDeviceCreateInfo deviceCreateInfo      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount    = static_cast<u32>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount   = static_cast<u32>(requiredDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

    if (bIsBindlessSupported)
    {
        deviceCreateInfo.pNext = &physical_features2;
    }

    if (validationLayersEnabled)
    {
        deviceCreateInfo.enabledLayerCount   = static_cast<u32>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        deviceCreateInfo.enabledLayerCount   = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
    }

    VkResult ok = vkCreateDevice(Physical_device, &deviceCreateInfo, nullptr, &Handle);
    vkcheck(ok);

    STRACE("\tRetrieving queue handles ...");
    vkGetDeviceQueue(Handle, GraphicsFamily, 0, &GraphicsQueue);

    STRACE("\tLogical device created!");

    return true;
}

void VulkanDevice::SetupDebugMessenger()
{
    if (!validationLayersEnabled)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugMessengerCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = debugCallback;
    debugMessengerCreateInfo.pUserData       = nullptr;

    if (CreateDebugUtilsMessengerEXT(Instance, &debugMessengerCreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to setup debug messenger!");
    }
}

bool VulkanDevice::CheckValidationLayersSupport()
{
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool found = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }
    return true;
}

u32 VulkanDevice::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags propertyFlags) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(Physical_device, &memoryProperties);

    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
        {
            return i;
        }
    }

    SASSERT_MSG(false, "Failed to find suitable memory type.");
    return 0;
}

void VulkanDevice::CreateSwapchain(GLFWwindow* window)
{
    SASSERT(window);

    if (swapchain == nullptr)
    {
        swapchain = std::make_shared<VulkanSwapchain>(this);
    }

    if (swapchain->surface == VK_NULL_HANDLE)
    {
        // Surface creation
        STRACE("\tCreating vulkan window surface handle ...");
        if (glfwCreateWindowSurface(Instance, window, nullptr, &swapchain->surface) != VK_SUCCESS)
        {
            SERROR("\tFailed to create VkSurface.");
            return;
        }
    }

    // Queue families
    u32 i = 0;
    for (const auto& queueFamily : queueFamilyProperties)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(Physical_device, i, swapchain->surface, &presentSupport);
        if (PresentFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && presentSupport)
        {
            PresentFamily = i;
        }
        i++;
    }

    if (!VulkanSwapchain::Create(this, swapchain))
    {
        SERROR("Failed to create vulkan swapchain");
    }
}

VulkanBuffer* VulkanDevice::GetBufferResource(BufferHandle handle)
{
    return static_cast<VulkanBuffer*>(buffers.AccessResource(handle.index));
}

VulkanShaderState* VulkanDevice::GetShaderResource(ShaderStateHandle handle)
{
    return static_cast<VulkanShaderState*>(shaders.AccessResource(handle.index));
}

VulkanSampler* VulkanDevice::GetSamplerResource(SamplerHandle handle)
{
    return static_cast<VulkanSampler*>(shaders.AccessResource(handle.index));
}

VulkanTexture* VulkanDevice::GetTextureResource(TextureHandle handle)
{
    return static_cast<VulkanTexture*>(textures.AccessResource(handle.index));
}

VulkanDescriptorSet* VulkanDevice::GetDescriptorSetResource(DescriptorSetHandle handle)
{
    return static_cast<VulkanDescriptorSet*>(descriptorSets.AccessResource(handle.index));
}

VulkanDescriptorSetLayout* VulkanDevice::GetDescriptorSetLayoutResource(DescriptorSetLayoutHandle handle)
{
    return static_cast<VulkanDescriptorSetLayout*>(descriptorSetLayouts.AccessResource(handle.index));
}

VulkanPipeline* VulkanDevice::GetPipelineResource(PipelineHandle handle)
{
    return static_cast<VulkanPipeline*>(pipelines.AccessResource(handle.index));
}

VulkanRenderPass* VulkanDevice::GetRenderPassResource(RenderPassHandle handle)
{
    return static_cast<VulkanRenderPass*>(renderpasses.AccessResource(handle.index));
}

void VulkanDevice::DestroyBufferInstant(ResourceHandle InHandle)
{
    auto buffer = static_cast<VulkanBuffer*>(buffers.AccessResource(InHandle));

    if (buffer)
    {
        buffer->Release();
    }

    buffers.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyTextureInstant(ResourceHandle InHandle)
{
    VulkanTexture* texture = static_cast<VulkanTexture*>(textures.AccessResource(InHandle));

    if (texture)
    {
        vkFreeMemory(Handle, texture->memory, nullptr);
        vkDestroyImageView(Handle, texture->image_view, nullptr);
        vkDestroyImage(Handle, texture->texture, nullptr);
    }

    textures.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyShaderStateInstant(ResourceHandle InHandle)
{
    auto shader = static_cast<VulkanShaderState*>(shaders.AccessResource(InHandle));

    if (shader)
    {
        for (u32 i = 0; i < shader->ActiveShaders; ++i)
        {
            vkDestroyShaderModule(Handle, shader->ShaderStageInfo[i].module, nullptr);
        }
    }

    shaders.ReleaseResource(InHandle);
}

void VulkanDevice::DestroySamplerInstant(ResourceHandle InHandle)
{
    auto sampler = static_cast<VulkanSampler*>(samplers.AccessResource(InHandle));

    if (sampler)
    {
        vkDestroySampler(Handle, sampler->sampler, nullptr);
    }

    samplers.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyDescriptorSetInstant(ResourceHandle InHandle)
{
    auto descriptor_set = static_cast<VulkanDescriptorSet*>(descriptorSets.AccessResource(InHandle));

    if (descriptor_set)
    {
        allocator->deallocate(descriptor_set->resources);
        descriptor_set->resources_count = 0;
    }

    descriptorSets.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyDescriptorSetLayoutInstant(ResourceHandle InHandle)
{
    auto descriptor_set_layout = static_cast<VulkanDescriptorSetLayout*>(descriptorSetLayouts.AccessResource(InHandle));

    if (descriptor_set_layout)
    {
        vkDestroyDescriptorSetLayout(Handle, descriptor_set_layout->descriptor_set_layout, nullptr);
        allocator->deallocate(descriptor_set_layout->bindings);
    }

    descriptorSetLayouts.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyPipelineInstant(ResourceHandle InHandle)
{
    auto pipeline = static_cast<VulkanPipeline*>(pipelines.AccessResource(InHandle));

    if (pipeline)
    {
        vkDestroyPipeline(Handle, pipeline->pipeline, nullptr);
        vkDestroyPipelineLayout(Handle, pipeline->pipelineLayout, nullptr);
    }

    pipelines.ReleaseResource(InHandle);
}

void VulkanDevice::DestroyRenderPassInstant(ResourceHandle InHandle)
{
    auto render_pass = static_cast<VulkanRenderPass*>(renderpasses.AccessResource(InHandle));

    if (render_pass)
    {
        if (render_pass->render_targets_count > 0)
        {
            vkDestroyFramebuffer(Handle, render_pass->framebuffer, nullptr);
        }

        // This should be destroyed with the renderpass static map ... avoid duplicate destruction calls.
    }

    renderpasses.ReleaseResource(InHandle);
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
