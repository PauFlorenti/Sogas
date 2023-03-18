
#include "vulkan/vulkan_device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_attachment.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_commandbuffer.h"
#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_renderpass.h"
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

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

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

VulkanDevice::VulkanDevice(GraphicsAPI apiType,
                           void* /*device*/,
                           std::vector<const char*> extensions,
                           Memory::Allocator*       InAllocator)
: glfwExtensions(std::move(extensions))
{
    api_type = apiType;

    allocator = InAllocator;
}

VulkanDevice::~VulkanDevice()
{
    shutdown();
}

void VulkanDevice::CreateSwapchain(std::shared_ptr<Renderer::Swapchain> swapchain, GLFWwindow* window)
{
    SASSERT(window);

    if (swapchain->internalState == nullptr)
    {
        swapchain->internalState = new VulkanSwapchain(this);
    }
    auto internalState = VulkanSwapchain::ToInternal(swapchain);

    if (internalState->surface == VK_NULL_HANDLE)
    {
        // Surface creation
        STRACE("\tCreating vulkan window surface handle ...");
        if (glfwCreateWindowSurface(Instance, window, nullptr, &internalState->surface) != VK_SUCCESS)
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
        vkGetPhysicalDeviceSurfaceSupportKHR(Physical_device, i, internalState->surface, &presentSupport);
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

bool VulkanDevice::Init()
{
    STRACE("Initializing Vulkan renderer ... ");

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
    textures.Init(allocator, 8, sizeof(VulkanTexture));

    CreateCommandResources();

    STRACE("Finished Initializing Vulkan device.\n");

    return true;
}

void VulkanDevice::shutdown()
{
    STRACE("Shutting down Vulkan renderer ...");

    STRACE("\tDestroying Command Pool ...");
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyCommandPool(Handle, resourcesCommandPool[i], nullptr);
        vkDestroyFence(Handle, fence[i], nullptr);
    }

    commandBuffers.clear();

    vkDestroySemaphore(Handle, beginSemaphore, nullptr);
    vkDestroySemaphore(Handle, endSemaphore, nullptr);

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

CommandBuffer VulkanDevice::BeginCommandBuffer()
{
    u32 count = commandBufferCounter++;
    if (count >= commandBuffers.size())
    {
        commandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(this));
    }

    CommandBuffer cmd;
    cmd.internalState = commandBuffers[count].get();
    auto internalCmd  = VulkanCommandBuffer::ToInternal(&cmd);

    if (internalCmd->commandBuffers[GetFrameIndex()] == VK_NULL_HANDLE)
    {
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            commandPoolInfo.queueFamilyIndex        = GraphicsFamily; // TODO make it configurable
            commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            if (vkCreateCommandPool(Handle, &commandPoolInfo, nullptr, &internalCmd->commandPools[i]) != VK_SUCCESS)
            {
                SERROR("Failed to create command pool.");
                return {};
            }

            VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            allocInfo.commandBufferCount          = 1;
            allocInfo.commandPool                 = VulkanCommandBuffer::ToInternal(&cmd)->commandPools[i];
            allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            if (vkAllocateCommandBuffers(Handle, &allocInfo, &internalCmd->commandBuffers[i]) != VK_SUCCESS)
            {
                SERROR("Failed to allocate command buffer");
                return {};
            }

            VkDescriptorPoolSize poolSize;
            poolSize.descriptorCount = 1;
            poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            descriptorPoolInfo.maxSets                    = 1;
            descriptorPoolInfo.poolSizeCount              = 1;
            descriptorPoolInfo.pPoolSizes                 = &poolSize;

            if (vkCreateDescriptorPool(Handle, &descriptorPoolInfo, nullptr, &internalCmd->descriptorPools[i]) !=
                VK_SUCCESS)
            {
                SERROR("Failed to allocate descriptor pool.");
                return {};
            }
        }

        VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(Handle, &semaphoreInfo, nullptr, &internalCmd->semaphore);
    }

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    if (vkBeginCommandBuffer(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()], &beginInfo) !=
        VK_SUCCESS)
    {
        SERROR("Begin command buffer failed.");
        return {};
    }

    return cmd;
}

void VulkanDevice::SubmitCommandBuffers()
{
    // TODO submit frame resources commands

    commandBufferCounter = 0;
    std::vector<VkCommandBuffer>         submitCommands;
    std::vector<VkSemaphore>             signalSemaphores;
    std::vector<VkSemaphore>             waitSemaphores;
    std::vector<VkSemaphore>             presentWaitSemaphores;
    std::shared_ptr<Renderer::Swapchain> swapchain;
    std::vector<VkPipelineStageFlags>    waitStages = {};

    std::vector<VkFence> fences;

    for (const auto& cmd : commandBuffers)
    {
        if (vkEndCommandBuffer(cmd->commandBuffers[GetFrameIndex()]) != VK_SUCCESS)
        {
            SFATAL("Failed to end command buffer");
            SASSERT_MSG(false, "Failed to end command buffer.");
        }

        if (cmd->swapchain)
        {
            swapchain                   = cmd->swapchain;
            auto swapchainInternalState = VulkanSwapchain::ToInternal(swapchain);
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            waitSemaphores.push_back(swapchainInternalState->presentCompleteSemaphore);
            signalSemaphores.push_back(swapchainInternalState->renderCompleteSemaphore);
        }

        submitCommands.push_back(cmd->commandBuffers[GetFrameIndex()]);
        signalSemaphores.push_back(cmd->semaphore);

        if (!cmd->commandsToWait.empty())
        {
            for (auto& cmdToWait : cmd->commandsToWait)
            {
                auto commandInternalState = VulkanCommandBuffer::ToInternal(&cmdToWait);
                waitSemaphores.push_back(commandInternalState->semaphore);
                waitStages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            }
            cmd->commandsToWait.clear();
        }

        VkSubmitInfo submitInfo         = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount   = static_cast<u32>(submitCommands.size());
        submitInfo.pCommandBuffers      = submitCommands.data();
        submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
        submitInfo.pSignalSemaphores    = signalSemaphores.data();
        submitInfo.waitSemaphoreCount   = static_cast<u32>(waitSemaphores.size());
        submitInfo.pWaitSemaphores      = waitSemaphores.data();
        submitInfo.pWaitDstStageMask    = waitStages.data();

        VkFence           f;
        VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(Handle, &fenceInfo, nullptr, &f);
        fences.push_back(f);

        if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, fences.back()) != VK_SUCCESS)
        {
            SASSERT_MSG(false, "Failed to submit graphics commands");
        }

        if (cmd == commandBuffers.back())
        {
            presentWaitSemaphores = signalSemaphores;
        }

        waitStages.clear();
        submitCommands.clear();
        waitSemaphores.clear();
        signalSemaphores.clear();
    }

    if (swapchain)
    {
        auto swapchainInternalState = VulkanSwapchain::ToInternal(swapchain);

        VkPresentInfoKHR presentInfo   = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &swapchainInternalState->swapchain;
        presentInfo.waitSemaphoreCount = static_cast<u32>(presentWaitSemaphores.size());
        presentInfo.pWaitSemaphores    = presentWaitSemaphores.data();
        presentInfo.pImageIndices      = &swapchainInternalState->imageIndex;

        VkResult res = vkQueuePresentKHR(GraphicsQueue, &presentInfo);

        if (res != VK_SUCCESS)
        {
            if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
            {
                swapchain->resized = true;
                SASSERT(VulkanSwapchain::Create(this, swapchain));
            }
            else
            {
                SASSERT_MSG(false, "Failed to present image.");
            }
        }
    }

    FrameCount++;

    vkWaitForFences(Handle, static_cast<u32>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);

    for (auto& f : fences)
    {
        vkDestroyFence(Handle, f, nullptr);
    }
}

void VulkanDevice::BeginRenderPass(std::shared_ptr<Renderer::Swapchain> InSwapchain, CommandBuffer cmd)
{
    auto internalCommand              = VulkanCommandBuffer::ToInternal(&cmd);
    auto internalSwapchain            = VulkanSwapchain::ToInternal(InSwapchain);
    internalCommand->activeRenderPass = InSwapchain->GetRenderpass();
    internalCommand->swapchain        = InSwapchain;

    VkResult res = vkAcquireNextImageKHR(Handle,
                                         internalSwapchain->swapchain,
                                         UINT64_MAX,
                                         internalSwapchain->presentCompleteSemaphore,
                                         VK_NULL_HANDLE,
                                         &internalSwapchain->imageIndex);
    if (res != VK_SUCCESS)
    {
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            InSwapchain->resized = true;
            if (VulkanSwapchain::Create(this, InSwapchain))
            {
                BeginRenderPass(InSwapchain, cmd);
                return;
            }
        }
        throw std::runtime_error("Failed to acquire next image!");
    }

    if (InSwapchain->resized)
    {
        return;
        // i32 width, height;
        // glfwGetWindowSize(window, &width, &height);
        // //CApplication::Get()->GetWindowSize(&width, &height);
        // InSwapchain->SetSwapchainSize(width, height);
        // InSwapchain->resized = false;
    }

    // Begin Render Pass
    const auto& swapchain_descriptor = InSwapchain->GetDescriptor();
    VkViewport  viewport{};
    viewport.x        = 0;
    viewport.y        = (f32)swapchain_descriptor.height;
    viewport.width    = (f32)swapchain_descriptor.width;
    viewport.height   = -(f32)swapchain_descriptor.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(internalCommand->commandBuffers[GetFrameIndex()], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width  = swapchain_descriptor.width;
    scissor.extent.height = swapchain_descriptor.height;
    scissor.offset        = {0, 0};

    vkCmdSetScissor(internalCommand->commandBuffers[GetFrameIndex()], 0, 1, &scissor);

    VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderpassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderpassBeginInfo.renderPass            = VulkanRenderPass::ToInternal(internalCommand->activeRenderPass)->renderpass;
    renderpassBeginInfo.clearValueCount       = 1;
    renderpassBeginInfo.pClearValues          = &clearValue;
    renderpassBeginInfo.framebuffer           = internalSwapchain->framebuffers.at(internalSwapchain->imageIndex);
    renderpassBeginInfo.renderArea.extent     = internalSwapchain->extent;
    renderpassBeginInfo.renderArea.offset     = {0, 0};

    vkCmdBeginRenderPass(internalCommand->commandBuffers[GetFrameIndex()],
                         &renderpassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanDevice::BeginRenderPass(Renderer::RenderPass* InRenderpass, CommandBuffer cmd)
{
    SASSERT(InRenderpass);

    auto commandBufferInternalState              = VulkanCommandBuffer::ToInternal(&cmd);
    auto renderpassInternalState                 = VulkanRenderPass::ToInternal(InRenderpass);
    commandBufferInternalState->activeRenderPass = InRenderpass;

    VkViewport viewport{};
    viewport.x        = 0;
    viewport.y        = (f32)480;
    viewport.width    = (f32)640;
    viewport.height   = -(f32)480;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBufferInternalState->commandBuffers[GetFrameIndex()], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width  = 640;
    scissor.extent.height = 480;
    scissor.offset        = {0, 0};

    vkCmdSetScissor(commandBufferInternalState->commandBuffers[GetFrameIndex()], 0, 1, &scissor);

    vkCmdBeginRenderPass(commandBufferInternalState->commandBuffers[GetFrameIndex()],
                         &renderpassInternalState->beginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanDevice::EndRenderPass(CommandBuffer cmd)
{
    auto internalCmd = VulkanCommandBuffer::ToInternal(&cmd);
    SASSERT(internalCmd);

    vkCmdEndRenderPass(internalCmd->commandBuffers[GetFrameIndex()]);
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
    return SamplerHandle();
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
    auto buffer = static_cast<VulkanBuffer*>(buffers.AccessResource(InHandle.index));

    if (buffer)
    {
        buffer->Release();
    }

    buffers.ReleaseResource(InHandle.index);
}

void VulkanDevice::DestroyTexture(TextureHandle InHandle)
{
    VulkanTexture* texture = static_cast<VulkanTexture*>(textures.AccessResource(InHandle.index));

    if (texture)
    {
        vkFreeMemory(Handle, texture->memory, nullptr);
        vkDestroyImageView(Handle, texture->image_view, nullptr);
        vkDestroyImage(Handle, texture->texture, nullptr);
        //texture->Release();
    }

    textures.ReleaseResource(InHandle.index);
}
void VulkanDevice::DestroyShaderState(ShaderStateHandle InHandle)
{
}
void VulkanDevice::DestroySampler(SamplerHandle InHandle)
{
}
void VulkanDevice::DestroyDescriptorSet(DescriptorSetHandle InHandle)
{
}
void VulkanDevice::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle InHandle)
{
}
void VulkanDevice::DestroyPipeline(PipelineHandle InPipInHandleeline)
{
}
void VulkanDevice::DestroyRenderPass(RenderPassHandle InHandle)
{
}

std::shared_ptr<Renderer::Buffer> VulkanDevice::CreateBuffer(Renderer::BufferDescriptor desc, void* data) const
{
    return VulkanBuffer::Create(this, desc, data);
}

std::shared_ptr<Renderer::Buffer> VulkanDevice::CreateBuffer(const u32&                   size,
                                                             const u64&                   element_size,
                                                             Renderer::BufferBindingPoint binding,
                                                             Renderer::BufferUsage        usage,
                                                             void*                        data) const
{
    Renderer::BufferDescriptor buffer_descriptor{};
    buffer_descriptor.size        = size;
    buffer_descriptor.elementSize = element_size;
    buffer_descriptor.binding     = binding;
    buffer_descriptor.usage       = usage;
    buffer_descriptor.type        = Renderer::BufferType::Static;
    return VulkanBuffer::Create(this, std::move(buffer_descriptor), data);
}

void VulkanDevice::CreateTexture(Texture* texture, void* data) const
{
    VulkanTexture::Create(this, texture, data);
}

std::shared_ptr<Texture> VulkanDevice::CreateTexture(TextureDescriptor desc, void* data) const
{
    return VulkanTexture::Create(this, std::move(desc), data);
};

void VulkanDevice::CreateRenderPass(Renderer::RenderPass* renderpass) const
{
    VulkanRenderPass::Create(this, renderpass);
}

void VulkanDevice::CreatePipeline(const PipelineDescriptor* desc,
                                  Pipeline*                 pipeline,
                                  Renderer::RenderPass*     renderpass) const
{
    VulkanPipeline::Create(this, desc, pipeline, renderpass);
}

void VulkanDevice::CreateShader(ShaderStageType InStageType, std::string filename, Shader* shader) const
{
    VulkanShader::Create(this, InStageType, std::move(filename), shader);
}

void VulkanDevice::UpdateDescriptorSet(const Pipeline* InPipeline) const
{
    auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
    for (auto& descriptorSet : pipelineInternalState->descriptorSets[GetFrameIndex()])
    {
        auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&descriptorSet);
        if (descriptorSet.dirty)
        {
            vkUpdateDescriptorSets(Handle,
                                   static_cast<u32>(descriptorSetInternalState->writes.size()),
                                   descriptorSetInternalState->writes.data(),
                                   0,
                                   nullptr);
            descriptorSet.dirty = false;
        }
    }
}

void VulkanDevice::CreateAttachment(AttachmentFramebuffer* InAttachment) const
{
    SASSERT(InAttachment);
    // return VulkanTexture::Create(this, , nullptr);
    VulkanAttachment::Create(this, InAttachment);
}

void VulkanDevice::SetWindowSize(std::shared_ptr<Renderer::Swapchain> InSwapchain, const u32& width, const u32& height)
{
    const auto& swapchain_descriptor = InSwapchain->GetDescriptor();
    if (InSwapchain->resized || (swapchain_descriptor.width != width, swapchain_descriptor.height != height))
    {
        InSwapchain->SetSwapchainSize(width, height);
        InSwapchain->resized = false;
    }
}

void VulkanDevice::BindVertexBuffer(const std::shared_ptr<Renderer::Buffer>& buffer, CommandBuffer cmd)
{
    SASSERT(buffer);
    SASSERT(buffer->isValid());

    const auto internalBuffer = VulkanBuffer::ToInternal(buffer->device_buffer);
    SASSERT(internalBuffer);

    VkDeviceSize offset = {0};
    vkCmdBindVertexBuffers(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()],
                           0,
                           1,
                           internalBuffer->GetHandle(),
                           &offset);
}

void VulkanDevice::BindIndexBuffer(const std::shared_ptr<Renderer::Buffer>& buffer, CommandBuffer cmd)
{
    SASSERT(buffer->isValid());

    const auto internalBuffer = VulkanBuffer::ToInternal(buffer->device_buffer);
    SASSERT(internalBuffer);

    VkDeviceSize offset = {0};
    vkCmdBindIndexBuffer(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()],
                         *internalBuffer->GetHandle(),
                         offset,
                         VK_INDEX_TYPE_UINT32);
}

void VulkanDevice::BindPipeline(const Pipeline* InPipeline, CommandBuffer& cmd)
{
    SASSERT(InPipeline)
    auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
    cmd.activePipeline         = InPipeline;

    vkCmdBindPipeline(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipelineInternalState->pipeline);
}

void VulkanDevice::BindDescriptor(CommandBuffer cmd)
{
    auto commandBufferInternalState = VulkanCommandBuffer::ToInternal(&cmd);
    auto pipelineInternalState      = VulkanPipeline::ToInternal(cmd.activePipeline);

    for (auto& descriptorSet : pipelineInternalState->descriptorSets[GetFrameIndex()])
    {
        auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&descriptorSet);
        descriptorSetInternalState->BindDescriptor(commandBufferInternalState->commandBuffers[GetFrameIndex()]);
    }
}

void VulkanDevice::BindBuffer(const std::shared_ptr<Renderer::Buffer>& InBuffer,
                              const Pipeline*                          InPipeline,
                              const u32                                InSlot,
                              const u32                                InDescriptorSet,
                              const u32                                InOffset)
{
    auto bufferInternalState   = VulkanBuffer::ToInternal(InBuffer->device_buffer);
    auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
    auto descriptorSetInternalState =
      VulkanDescriptorSet::ToInternal(&pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet));

    bufferInternalState->descriptorInfo.buffer = *bufferInternalState->GetHandle();
    bufferInternalState->descriptorInfo.offset = InOffset;
    bufferInternalState->descriptorInfo.range  = InBuffer->ByteSize();

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorCount      = 1;
    write.dstBinding           = InSlot;
    write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.dstSet               = descriptorSetInternalState->GetDescriptorSet();
    write.pBufferInfo          = &bufferInternalState->descriptorInfo;

    descriptorSetInternalState->writes.push_back(write);

    pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet).dirty = true;
}

void VulkanDevice::BindTexture(const Texture*  InTexture,
                               const Pipeline* InPipeline,
                               const u32       InSlot,
                               const u32       InDescriptorSet)
{
    auto internalState         = VulkanTexture::ToInternal(InTexture);
    auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
    auto descriptorSetInternalState =
      VulkanDescriptorSet::ToInternal(&pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet));

    internalState->descriptorImageInfo.imageView   = internalState->GetImageView();
    internalState->descriptorImageInfo.sampler     = internalState->GetSampler();
    internalState->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorCount      = 1;
    write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.dstBinding           = InSlot;
    write.dstSet               = descriptorSetInternalState->GetDescriptorSet();
    write.pImageInfo           = &internalState->descriptorImageInfo;

    descriptorSetInternalState->writes.push_back(write);
    pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet).dirty = true;
}

void VulkanDevice::BindAttachment(const AttachmentFramebuffer* InAttachment,
                                  const Pipeline*              InPipeline,
                                  const u32                    InSlot,
                                  const u32                    InDescriptorSet)
{
    auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
    auto descriptorSetInternalState =
      VulkanDescriptorSet::ToInternal(&pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet));
    auto attachmentInternalState = VulkanAttachment::ToInternal(InAttachment);

    attachmentInternalState->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachmentInternalState->imageInfo.imageView   = attachmentInternalState->GetImageView();
    attachmentInternalState->imageInfo.sampler     = attachmentInternalState->GetSampler();

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorCount      = 1;
    write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.dstBinding           = InSlot;
    write.dstSet               = descriptorSetInternalState->GetDescriptorSet();
    write.pImageInfo           = &attachmentInternalState->imageInfo;

    descriptorSetInternalState->writes.push_back(write);
    pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet).dirty = true;
}

void VulkanDevice::SetTopology(PrimitiveTopology topology)
{
    VkCommandBuffer& cmd = resourcesCommandBuffer[GetFrameIndex()];
    switch (topology)
    {
        case PrimitiveTopology::POINTLIST:
            vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
            break;
        case PrimitiveTopology::LINELIST:
            vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
            break;
        case PrimitiveTopology::TRIANGLELIST:
            vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            break;
        default:
            SERROR("No valid topology!");
            break;
    }
}

void VulkanDevice::Draw(const u32 count, const u32 offset, CommandBuffer cmd)
{
    SASSERT(count > 0);
    SASSERT(offset >= 0);

    vkCmdDraw(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()], count, 1, offset, 0);
}

void VulkanDevice::DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd)
{
    SASSERT(count > 0);
    SASSERT(offset >= 0);

    vkCmdDrawIndexed(VulkanCommandBuffer::ToInternal(&cmd)->commandBuffers[GetFrameIndex()], count, 1, offset, 0, 0);
}

void VulkanDevice::PushConstants(const void* InData, const u32 InSize, const CommandBuffer cmd)
{
    auto vkcmd = VulkanCommandBuffer::ToInternal(&cmd);
    vkCmdPushConstants(vkcmd->commandBuffers[GetFrameIndex()],
                       VulkanPipeline::ToInternal(cmd.activePipeline)->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       InSize,
                       InData);
}

void VulkanDevice::UpdateBuffer(const std::shared_ptr<Renderer::Buffer>& InBuffer,
                                const void*                              InData,
                                const u32                                InDataSize,
                                const u32                                InOffset,
                                CommandBuffer /*cmd*/)
{
    auto  bufferInternalState = VulkanBuffer::ToInternal(InBuffer->device_buffer);
    void* data;
    vkMapMemory(Handle, bufferInternalState->GetMemory(), InOffset, InDataSize, 0, &data);
    std::memcpy(data, InData, InDataSize);
    vkUnmapMemory(Handle, bufferInternalState->GetMemory());
}

void VulkanDevice::WaitCommand(CommandBuffer& cmd, CommandBuffer& cmdToWait)
{
    auto commandInternalState = VulkanCommandBuffer::ToInternal(&cmd);
    commandInternalState->commandsToWait.push_back(cmdToWait);
}

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
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            GraphicsFamily = i;
        }
        i++;
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

void VulkanDevice::CreateCommandResources()
{
    STRACE("\tCreating Command Pool ...");
    VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    createInfo.queueFamilyIndex        = GraphicsFamily;
    createInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateCommandPool(Handle, &createInfo, nullptr, &resourcesCommandPool[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandPool                 = resourcesCommandPool[i];
        allocInfo.commandBufferCount          = 1;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(Handle, &allocInfo, &resourcesCommandBuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allcoate command buffer!");
        }

        VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

        if (vkCreateFence(Handle, &fenceInfo, nullptr, &fence[i]) != VK_SUCCESS)
        {
            SFATAL("Failed to create frame fence.");
            return;
        }
    }
}

VkRenderPass VulkanDevice::GetVulkanRenderPass(const RenderPassOutput& InOutput, std::string InName)
{
    u64          hashed      = wyhash((void*)&InOutput, sizeof(RenderPassOutput), 0, _wyp);
    VkRenderPass render_pass = render_pass_cache.at(hashed);
    if (render_pass)
    {
        return render_pass;
    }

    render_pass = VulkanRenderPass::CreateRenderPass(this, InOutput, InName);
    render_pass_cache.insert(std::pair<u64, VkRenderPass>(hashed, render_pass));

    return render_pass;
}

VulkanBuffer* VulkanDevice::GetBufferResource(BufferHandle handle)
{
    return static_cast<VulkanBuffer*>(buffers.AccessResource(handle.index));
}

VulkanShaderState* VulkanDevice::GetShaderResource(ShaderStateHandle handle)
{
    return static_cast<VulkanShaderState*>(shaders.AccessResource(handle.index));
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

} // namespace Vk
} // namespace Renderer
} // namespace Sogas
