
#include "render/vulkan/vulkan_device.h"

#include "application.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "render/vulkan/vulkan_buffer.h"
#include "render/vulkan/vulkan_commandbuffer.h"
#include "render/vulkan/vulkan_descriptorSet.h"
#include "render/vulkan/vulkan_pipeline.h"
#include "render/vulkan/vulkan_renderpass.h"
#include "render/vulkan/vulkan_shader.h"
#include "render/vulkan/vulkan_types.h"
#include "render/vulkan/vulkan_texture.h"
#include "render/vulkan/vulkan_swapchain.h"

namespace Sogas
{
    namespace Vk
    {
        const std::vector<const char *> validationLayers = {
            "VK_LAYER_KHRONOS_validation"};

        const std::vector<const char *> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
        const bool validationLayersEnabled = false;
#else
        const bool validationLayersEnabled = true;
#endif

        static std::vector<const char *> GetRequiredExtensions()
        {
            // Getting GLFW required instance extensions.
            u32 glfwExtensionCount = 0;
            const char **glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (validationLayersEnabled)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugUtilsMessengerEXT *pDebugMessenger)
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

        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks *pAllocator)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                func(instance, debugMessenger, pAllocator);
            }
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT /*type*/,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void * /*pUserData*/)
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
            vkGetPhysicalDeviceMemoryProperties(Gpu, &memoryProperties);

            for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
            {
                if ((typeFilter & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
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
            vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);

            if (PhysicalDeviceCount == 0)
            {
                throw std::runtime_error("No available physical device!");
            }

            std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
            vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

            STRACE("Available Physical Devices ...");

            for (const auto gpu : PhysicalDevices)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(gpu, &properties);
                STRACE("\tDevice Name: %s\tVendor ID: %i", properties.deviceName, properties.vendorID);
            }

            std::cout << "\t--\t--\t--\n\n";

            Gpu = PhysicalDevices.at(0);

            // TODO Make a proper Physical device selector ...
            // Is Device suitable ??
        }

        VulkanDevice::VulkanDevice(GraphicsAPI apiType, void * /*device*/)
        {
            api_type = apiType;
        }

        VulkanDevice::~VulkanDevice()
        {
        }

        void VulkanDevice::CreateSwapchain(const SwapchainDescriptor& desc, Swapchain* swapchain)
        {
            auto internalState = std::static_pointer_cast<VulkanSwapchain>(swapchain->internalState);

            if (swapchain->internalState == nullptr)
            {
                internalState = std::make_shared<VulkanSwapchain>();
            }

            swapchain->descriptor       = desc;
            swapchain->internalState    = internalState;

            if (internalState->surface == VK_NULL_HANDLE)
            {
                // Surface creation
                STRACE("\tCreating vulkan window surface handle ...");
                if (glfwCreateWindowSurface(Instance, CApplication::Get()->GetWindow(), nullptr, &internalState->surface) != VK_SUCCESS)
                {
                    SERROR("\tFailed to create VkSurface.");
                    return;
                }
            }

            // Queue families
            u32 i = 0;
            for (const auto &queueFamily : queueFamilyProperties)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(Gpu, i, internalState->surface, &presentSupport);
                if (PresentFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && presentSupport)
                {
                    PresentFamily = i;
                }
                i++;
            }

            if (!VulkanSwapchain::Create(Handle, Gpu, &swapchain->descriptor, internalState.get()))
            {
                SERROR("Failed to create vulkan swapchain");
            }

            swapchain->renderpass.internalState = std::make_shared<VulkanRenderPass>();
            auto renderpassInternalState = std::static_pointer_cast<VulkanRenderPass>(swapchain->renderpass.internalState);
            auto swapchainRenderpassInternalState = std::static_pointer_cast<VulkanRenderPass>(internalState->renderpass.internalState);
            renderpassInternalState->renderpass = swapchainRenderpassInternalState->renderpass;
        }

        bool VulkanDevice::Init()
        {
            STRACE("Initializing Vulkan renderer ... ");

            if (!CreateInstance()) {
                STRACE("\tFailed to create Vulkan Instance!");
                return false;
            }

            if (!CreateDevice()) {
                STRACE("\tFailed to create Vulkan Logical Device!");
                return false;
            }

            CreateCommandResources();

            STRACE("Finished Initializing Vulkan device.\n");

            return true;
        }

        void VulkanDevice::shutdown()
        {
            STRACE("Shutting down Vulkan renderer ...");

            STRACE("\tDestroying Command Pool ...");
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyCommandPool(Handle, resourcesCommandPool[i], nullptr);
            }

            STRACE("\tDestroying Framebuffer ...");
            /*for (auto &framebuffer : SwapchainFramebuffers)
            {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }

            STRACE("\tDestroying Graphics pipeline ...");
            vkDestroyPipeline(Handle, Pipeline, nullptr);
            STRACE("\tDestroying Render Pass ...");
            vkDestroyRenderPass(Handle, RenderPass, nullptr);
            STRACE("\tDestroying Graphics pipeline layout ...");
            vkDestroyPipelineLayout(Handle, PipelineLayout, nullptr);
            */

            /*
            STRACE("\tDestroying all images and image views ...");
            for (auto &imageView : SwapchainImageViews)
            {
                vkDestroyImageView(Device, imageView, nullptr);
            }
            for (auto &image : SwapchainImages)
            {
                vkDestroyImage(Device, image, nullptr);
            }

            vkDestroySwapchainKHR(Device, Swapchain, nullptr);
            STRACE("\tDestroying Vulkan logical device ...");
            vkDestroyDevice(Handle, nullptr);
            STRACE("\tLogical device destroyed.");
            STRACE("\tDestroying Surface ...");
            vkDestroySurfaceKHR(Instance, Surface, nullptr);
            */
            if (validationLayersEnabled)
                DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
            STRACE("\tDestroying instance ...");
            vkDestroyInstance(Instance, nullptr);
            STRACE("\tInstance destroyed.");
            STRACE("Vulkan renderer has shut down.\n");
        }

        CommandBuffer VulkanDevice::BeginCommandBuffer()
        {
            u32 count = commandBufferCounter++;
            if (count >= commandBuffers.size()) {
                commandBuffers.push_back(std::make_unique<VulkanCommandBuffer>());
            }

            CommandBuffer cmd;
            cmd.internalState = commandBuffers[count].get();

            VulkanCommandBuffer* internalCmd = static_cast<VulkanCommandBuffer*>(cmd.internalState);
            if (internalCmd->commandBuffers[GetFrameIndex()] == VK_NULL_HANDLE) {

                for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                    commandPoolInfo.queueFamilyIndex = GraphicsFamily; // TODO make it configurable
                    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

                    if (vkCreateCommandPool(Handle, &commandPoolInfo, nullptr, &internalCmd->commandPools[i]) != VK_SUCCESS) {
                        SERROR("Failed to create command pool.");
                        return {};
                    }

                    VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                    allocInfo.commandBufferCount = 1;
                    allocInfo.commandPool        = VulkanCommandBuffer::ToInternal(&cmd).commandPools[i];
                    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                    if (vkAllocateCommandBuffers(Handle, &allocInfo, &internalCmd->commandBuffers[i]) != VK_SUCCESS) {
                        SERROR("Failed to allocate command buffer");
                        return {};
                    }

                    VkDescriptorPoolSize poolSize;
                    poolSize.descriptorCount    = 1;
                    poolSize.type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                    VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                    descriptorPoolInfo.maxSets          = 1;
                    descriptorPoolInfo.poolSizeCount    = 1;
                    descriptorPoolInfo.pPoolSizes       = &poolSize;

                    if (vkCreateDescriptorPool(Handle, &descriptorPoolInfo, nullptr, &internalCmd->descriptorPools[i]) != VK_SUCCESS) {
                        SERROR("Failed to allocate descriptor pool.");
                        return {};
                    }
                }
            }

            VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

            if (vkBeginCommandBuffer(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], &beginInfo) != VK_SUCCESS) {
                SERROR("Begin command buffer failed.");
                return {};
            }

            return cmd;
        }

        void VulkanDevice::SubmitCommandBuffers()
        {
            // TODO submit frame resources commands

            u32 nCommands = commandBufferCounter;
            commandBufferCounter = 0;
            std::vector<VkCommandBuffer> submitCommands;
            std::vector<VkSemaphore>     signalSemaphores;
            std::vector<VkSemaphore>     waitSemaphores;
            std::vector<VkSwapchainKHR>  submitSwapchains;
            std::vector<Swapchain*>       updateSwapchains;
            std::vector<u32>             swapchainImageIndices;

            for (u32 i = 0; i < nCommands; i++)
            {
                if (vkEndCommandBuffer(commandBuffers.at(i)->commandBuffers[GetFrameIndex()]) != VK_SUCCESS) {
                    SFATAL("Failed to end command buffer");
                    SASSERT_MSG(false, "Failed to end command buffer.");
                }

                auto internalSwapchain = VulkanSwapchain::ToInternal(commandBuffers.at(i)->swapchain);
                submitCommands.push_back(commandBuffers.at(i)->commandBuffers[GetFrameIndex()]);
                signalSemaphores.push_back(internalSwapchain->swapchainEndSemaphore);
                waitSemaphores.push_back(internalSwapchain->swapchainStartSemaphore);
                submitSwapchains.push_back(internalSwapchain->swapchain);
                updateSwapchains.push_back(commandBuffers.at(i)->swapchain);
                swapchainImageIndices.push_back(internalSwapchain->imageIndex);
            }

            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo submitInfo         = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount   = static_cast<u32>(submitCommands.size());
            submitInfo.pCommandBuffers      = submitCommands.data();
            submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphores    = signalSemaphores.data();
            submitInfo.waitSemaphoreCount   = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphores      = waitSemaphores.data();
            submitInfo.pWaitDstStageMask    = &waitStages;

            if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, fence[GetFrameIndex()]) != VK_SUCCESS) { 
                SASSERT_MSG(false, "Failed to submit graphics commands");
            }

            // TODO Only if is a swapchain pass, at the moment only a swapchain pass
            VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
            presentInfo.swapchainCount      = static_cast<u32>(submitSwapchains.size());
            presentInfo.pSwapchains         = submitSwapchains.data();
            presentInfo.waitSemaphoreCount  = static_cast<u32>(signalSemaphores.size());
            presentInfo.pWaitSemaphores     = signalSemaphores.data();
            presentInfo.pImageIndices       = swapchainImageIndices.data();

            VkResult res = vkQueuePresentKHR(GraphicsQueue, &presentInfo);
            if ( res != VK_SUCCESS) 
            {
                if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    for (auto& swapchain : updateSwapchains)
                    {
                        auto internalSwapchainState = VulkanSwapchain::ToInternal(swapchain);
                        swapchain->resized = true;
                        SASSERT(VulkanSwapchain::Create(Handle, Gpu, &swapchain->descriptor, internalSwapchainState));
                    }
                }
                else
                {
                    SASSERT_MSG(false, "Failed to present image.");
                }
            }

            FrameCount++;

            // Ignore 2 first frames.
            if (GetFrameCount() >= MAX_FRAMES_IN_FLIGHT)
            {
                if (vkWaitForFences(Handle, 1, &fence[GetFrameIndex()], VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
                    SERROR("Failed waiting for fence.");
                    return;
                }

                if (vkResetFences(Handle, 1, &fence[GetFrameIndex()]) != VK_SUCCESS) {
                    SERROR("Failed to reset fence.");
                    return;
                }
            }
        }

        void VulkanDevice::BeginRenderPass(Swapchain* swapchain, CommandBuffer cmd)
        {
            SASSERT(cmd.IsValid());
            VulkanCommandBuffer* internalCommand = static_cast<VulkanCommandBuffer*>(cmd.internalState);
            auto internalSwapchain = std::static_pointer_cast<VulkanSwapchain>(swapchain->internalState);
            internalCommand->activeRenderPass    = &internalSwapchain->renderpass;
            internalCommand->swapchain           = swapchain;

            VkResult res = vkAcquireNextImageKHR(Handle, internalSwapchain->swapchain, UINT64_MAX, internalSwapchain->swapchainStartSemaphore, VK_NULL_HANDLE, &internalSwapchain->imageIndex);
            if (res != VK_SUCCESS)
            {
                if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    swapchain->resized = true;
                    if (VulkanSwapchain::Create(Handle, Gpu, &swapchain->descriptor, VulkanSwapchain::ToInternal(swapchain)))
                    {
                        BeginRenderPass(swapchain, cmd);
                        return;
                    }
                }
                throw std::runtime_error("Failed to acquire next image!");
            }

            if (swapchain->resized)
            {
                i32 width, height;
                CApplication::Get()->GetWindowSize(&width, &height);
                swapchain->SetSwapchainSize(width, height);
                swapchain->resized = false;
            }

            // Begin Render Pass
            VkViewport viewport{};
            viewport.x          = 0;
            viewport.y          = (f32)swapchain->descriptor.height;
            viewport.width      = (f32)swapchain->descriptor.width;
            viewport.height     = -(f32)swapchain->descriptor.height;
            viewport.minDepth   = 0.0f;
            viewport.maxDepth   = 1.0f;

            vkCmdSetViewport(internalCommand->commandBuffers[GetFrameIndex()], 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.extent.width = swapchain->descriptor.width;
            scissor.extent.height = swapchain->descriptor.height;
            scissor.offset = {0, 0};

            vkCmdSetScissor(internalCommand->commandBuffers[GetFrameIndex()], 0, 1, &scissor);

            VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

            VkRenderPassBeginInfo renderpassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            renderpassBeginInfo.renderPass          = std::static_pointer_cast<VulkanRenderPass>(internalCommand->activeRenderPass->internalState)->renderpass;
            renderpassBeginInfo.clearValueCount     = 1;
            renderpassBeginInfo.pClearValues        = &clearValue;
            renderpassBeginInfo.framebuffer         = internalSwapchain->framebuffers.at(internalSwapchain->imageIndex);
            renderpassBeginInfo.renderArea.extent   = internalSwapchain->extent;
            renderpassBeginInfo.renderArea.offset   = {0, 0};

            vkCmdBeginRenderPass(internalCommand->commandBuffers[GetFrameIndex()], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        void VulkanDevice::EndRenderPass(CommandBuffer cmd)
        {
            VulkanCommandBuffer* internalCmd = static_cast<VulkanCommandBuffer*>(cmd.internalState);
            SASSERT(internalCmd);

            vkCmdEndRenderPass(internalCmd->commandBuffers[GetFrameIndex()]);
        }

        void VulkanDevice::CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const
        {
            VulkanBuffer::Create(this, desc, data, buffer);
        }

        void VulkanDevice::CreateTexture(const TextureDescriptor* desc, void* data, Texture* texture) const
        {
            VulkanTexture::Create(this, desc, data, texture);
        }

        void VulkanDevice::CreateRenderPass(const RenderPassDescriptor* desc, RenderPass* renderpass) const
        {
            VulkanRenderPass::Create(this, desc, renderpass);
        }

        void VulkanDevice::CreatePipeline(const PipelineDescriptor* desc, Pipeline* pipeline, RenderPass* renderpass) const
        {
            VulkanPipeline::Create(this, desc, pipeline, renderpass);
        }

        void VulkanDevice::CreateShader(ShaderStage stage, const char* filename, Shader* shader) const
        {
            VulkanShader::Create(this, stage, filename, shader);
        }

        void VulkanDevice::UpdateDescriptorSet(const Pipeline* InPipeline) const
        {
            auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
            for (auto& descriptorSet : pipelineInternalState->descriptorSets[GetFrameIndex()])
            {
                auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&descriptorSet);
                if (descriptorSet.dirty) 
                {
                    vkUpdateDescriptorSets(Handle, static_cast<u32>(descriptorSetInternalState->writes.size()), descriptorSetInternalState->writes.data(), 0, nullptr);
                    descriptorSet.dirty = false;
                }
            }
        }

        void VulkanDevice::BindVertexBuffer(const GPUBuffer* buffer, CommandBuffer cmd)
        {
            SASSERT(buffer);
            SASSERT(buffer->IsValid());
            SASSERT(buffer->IsBuffer());

            const VulkanBuffer* internalBuffer = static_cast<VulkanBuffer*>(buffer->internalState.get());
            SASSERT(internalBuffer);

            VkDeviceSize offset = {0};
            vkCmdBindVertexBuffers(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], 0, 1, internalBuffer->GetHandle(), &offset);
        }

        void VulkanDevice::BindIndexBuffer(const GPUBuffer* buffer, CommandBuffer cmd)
        {
            SASSERT(buffer);
            SASSERT(buffer->IsValid());
            SASSERT(buffer->IsBuffer());

            const VulkanBuffer* internalBuffer = static_cast<VulkanBuffer*>(buffer->internalState.get());
            SASSERT(internalBuffer);

            VkDeviceSize offset = {0};
            vkCmdBindIndexBuffer(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], *internalBuffer->GetHandle(), offset, VK_INDEX_TYPE_UINT32);
        }

        void VulkanDevice::BindPipeline(const Pipeline* InPipeline, CommandBuffer& cmd)
        {
            SASSERT(InPipeline)
            auto pipelineInternalState = VulkanPipeline::ToInternal(InPipeline);
            cmd.activePipeline = InPipeline;

            vkCmdBindPipeline(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineInternalState->handle);
        }

        void VulkanDevice::BindDescriptor(CommandBuffer cmd)
        {
            auto commandBufferInternalState = VulkanCommandBuffer::ToInternal(&cmd);
            auto pipelineInternalState = VulkanPipeline::ToInternal(cmd.activePipeline);

            for (auto& descriptorSet : pipelineInternalState->descriptorSets[GetFrameIndex()])
            {
                auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&descriptorSet);
                descriptorSetInternalState->BindDescriptor(commandBufferInternalState.commandBuffers[GetFrameIndex()]);
            }
        }

        void VulkanDevice::BindBuffer(const GPUBuffer* InBuffer, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet, const u32 InOffset)
        {
            auto bufferInternalState        = VulkanBuffer::ToInternal(InBuffer);
            auto pipelineInternalState      = VulkanPipeline::ToInternal(InPipeline);
            auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet));

            bufferInternalState->descriptorInfo.buffer  = *bufferInternalState->GetHandle();
            bufferInternalState->descriptorInfo.offset  = InOffset;
            bufferInternalState->descriptorInfo.range   = InBuffer->descriptor.size;
            
            VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.descriptorCount   = 1;
            write.dstBinding        = InSlot;
            write.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.dstSet            = descriptorSetInternalState->GetDescriptorSet();
            write.pBufferInfo       = &bufferInternalState->descriptorInfo;

            descriptorSetInternalState->writes.push_back(write);

            pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet).dirty = true;
        }

        void VulkanDevice::BindTexture(const Texture* InTexture, const Pipeline* InPipeline, const u32 InSlot, const u32 InDescriptorSet)
        {
            auto internalState              = VulkanTexture::ToInternal(InTexture);
            auto pipelineInternalState      = VulkanPipeline::ToInternal(InPipeline);
            auto descriptorSetInternalState = VulkanDescriptorSet::ToInternal(&pipelineInternalState->descriptorSets[GetFrameIndex()].at(InDescriptorSet));

            internalState->descriptorImageInfo.imageView     = internalState->GetImageView();
            internalState->descriptorImageInfo.sampler       = internalState->GetSampler();
            internalState->descriptorImageInfo.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.descriptorCount   = 1;
            write.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.dstBinding        = InSlot;
            write.dstSet            = descriptorSetInternalState->GetDescriptorSet();
            write.pImageInfo        = &internalState->descriptorImageInfo;

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

            vkCmdDraw(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], count, 1, offset, 0);
        }

        void VulkanDevice::DrawIndexed(const u32 count, const u32 offset, CommandBuffer cmd)
        {
            SASSERT(count > 0);
            SASSERT(offset >= 0);

            vkCmdDrawIndexed(VulkanCommandBuffer::ToInternal(&cmd).commandBuffers[GetFrameIndex()], count, 1, offset, 0, 0);
        }

        void VulkanDevice::PushConstants(const void* InData, const u32 InSize, const CommandBuffer cmd)
        {
            VulkanCommandBuffer vkcmd = VulkanCommandBuffer::ToInternal(&cmd);
            vkCmdPushConstants(
                vkcmd.commandBuffers[GetFrameIndex()],
                VulkanPipeline::ToInternal(cmd.activePipeline)->pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0, InSize, InData);
        }

        void VulkanDevice::UpdateBuffer(const GPUBuffer* InBuffer, const void* InData, const u32 InDataSize, const u32 InOffset, CommandBuffer /*cmd*/)
        {
            auto bufferInternalState = VulkanBuffer::ToInternal(InBuffer);
            void* data;
            vkMapMemory(Handle, bufferInternalState->GetMemory(), InOffset, InDataSize, 0, &data);
            std::memcpy(data, InData, InDataSize);
            vkUnmapMemory(Handle, bufferInternalState->GetMemory());
        }

        bool VulkanDevice::CreateInstance()
        {

            STRACE("\tCreating the Vulkan Instance ...");

            VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
            applicationInfo.pApplicationName = "Sogas app hardcoded";
            applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
            applicationInfo.pEngineName = "Sogas Engine";
            applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
            applicationInfo.apiVersion = VK_API_VERSION_1_3;

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

            // Get required extensions for application.
            auto extensions = GetRequiredExtensions();

            // Print the selected extensions ...

            VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
            instanceCreateInfo.pApplicationInfo = &applicationInfo;
            instanceCreateInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
            instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
            instanceCreateInfo.enabledLayerCount = validationLayersEnabled ? static_cast<u32>(validationLayers.size()) : 0;
            instanceCreateInfo.ppEnabledLayerNames = validationLayersEnabled ? validationLayers.data() : nullptr;

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
            vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &queueFamilyCount, nullptr);
            queueFamilyProperties.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &queueFamilyCount, queueFamilyProperties.data());

            u32 i = 0;
            for (const auto &queueFamily : queueFamilyProperties)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    GraphicsFamily = i;
                }
                i++;
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<u32> uniqueQueueFamilies = {GraphicsFamily};

            f32 queuePriority = 1.0f;
            for (u32 queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                createInfo.queueCount       = 1;
                createInfo.queueFamilyIndex = queueFamily;
                createInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(createInfo);
                queueFamilies.push_back(queueFamily);
            }

            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.wideLines = VK_TRUE;

            VkDeviceCreateInfo deviceCreateInfo         = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
            deviceCreateInfo.queueCreateInfoCount       = static_cast<u32>(queueCreateInfos.size());
            deviceCreateInfo.pQueueCreateInfos          = queueCreateInfos.data();
            deviceCreateInfo.enabledExtensionCount      = static_cast<u32>(requiredDeviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames    = requiredDeviceExtensions.data();
            deviceCreateInfo.pEnabledFeatures           = &deviceFeatures;

            if (validationLayersEnabled)
            {
                deviceCreateInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
                deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else
            {
                deviceCreateInfo.enabledLayerCount = 0;
                deviceCreateInfo.ppEnabledLayerNames = nullptr;
            }

            VkResult ok = vkCreateDevice(Gpu, &deviceCreateInfo, nullptr, &Handle);
            if (ok != VK_SUCCESS)
            {
                SERROR("\tFailed to create logical device.");
                return false;
            }

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

            for (const char *layerName : validationLayers)
            {
                bool found = false;
                for (const auto &layerProperties : availableLayers)
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
                return;

            VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
            debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugMessengerCreateInfo.pfnUserCallback = debugCallback;
            debugMessengerCreateInfo.pUserData = nullptr;

            if (CreateDebugUtilsMessengerEXT(Instance, &debugMessengerCreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to seupt debug messenger!");
            }
        }

        void VulkanDevice::CreateCommandResources()
        {
            STRACE("\tCreating Command Pool ...");
            VkCommandPoolCreateInfo createInfo  = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            createInfo.queueFamilyIndex         = GraphicsFamily;
            createInfo.flags                    = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (vkCreateCommandPool(Handle, &createInfo, nullptr, &resourcesCommandPool[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create command pool!");
                }

                VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                allocInfo.commandPool           = resourcesCommandPool[i];
                allocInfo.commandBufferCount    = 1;
                allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                if (vkAllocateCommandBuffers(Handle, &allocInfo, &resourcesCommandBuffer[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allcoate command buffer!");
                }

                VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                
                if (vkCreateFence(Handle, &fenceInfo, nullptr, &fence[i]) != VK_SUCCESS) { 
                    SFATAL("Failed to create frame fence.");
                    return;
                }
            }
        }
} // Vk
} // Sogas
