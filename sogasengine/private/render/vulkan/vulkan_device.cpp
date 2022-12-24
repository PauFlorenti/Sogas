
#include "render/vulkan/vulkan_device.h"

#include "application.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "render/vulkan/vulkan_buffer.h"
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

        static std::vector<char> ReadFile(const std::string &filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            SASSERT_MSG(file, "No file provided with name '%s'", filename.c_str());
            SASSERT_MSG(file.is_open(), "Failed to open file '%s'", filename.c_str());

            size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();

            return buffer;
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

            internalState->descriptor = desc;

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

            if (!VulkanSwapchain::Create(Handle, Gpu, internalState.get()))
            {
                SERROR("Failed to create vulkan swapchain");
            }

            // ! All the following code should be moved out of here.

            /*
            CreateDescriptorSetLayout();
            CreateGraphicsPipeline();
            CreateFramebuffers();
            CreateCommandPool();
            CreateCommandBuffer();
            CreateSyncObjects();
            CreateUniformBuffer();
            CreateDescriptorPools();
            CreateDescriptorSets();
            */
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

            STRACE("Finished Initializing Vulkan device.\n");

            return true;
        }

        void VulkanDevice::shutdown()
        {
            STRACE("Shutting down Vulkan renderer ...");

            STRACE("\tDestroying Uniform buffers ...");
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer(Handle, UniformBuffers.at(i), nullptr);
                vkFreeMemory(Handle, UniformBufferMemory.at(i), nullptr);
            }

            STRACE("\tDestroying Sync objects ...");
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyFence(Handle, InFlightFence.at(i), nullptr);
                vkDestroySemaphore(Handle, RenderFinishedSemaphore.at(i), nullptr);
                vkDestroySemaphore(Handle, ImageAvailableSemaphore.at(i), nullptr);
            }

            STRACE("\tDestroying Command Pool ...");
            vkDestroyCommandPool(Handle, CommandPool, nullptr);

            STRACE("\tDestroying Framebuffer ...");
            /*for (auto &framebuffer : SwapchainFramebuffers)
            {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }*/

            STRACE("\tDestroying Graphics pipeline ...");
            vkDestroyPipeline(Handle, Pipeline, nullptr);
            STRACE("\tDestroying Render Pass ...");
            vkDestroyRenderPass(Handle, RenderPass, nullptr);
            STRACE("\tDestroying Graphics pipeline layout ...");
            vkDestroyPipelineLayout(Handle, PipelineLayout, nullptr);
            STRACE("\tDestroying Descriptor Pool ...");
            vkDestroyDescriptorPool(Handle, DescriptorPool, nullptr);
            STRACE("\tDestroying Descriptor set layout ...");
            vkDestroyDescriptorSetLayout(Handle, DescriptorSetLayout, nullptr);

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
            vkDestroyDevice(Device, nullptr);
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

        void VulkanDevice::CreateBuffer(const GPUBufferDescriptor* desc, void* data, GPUBuffer* buffer) const
        {
            VulkanBuffer::Create(*this, desc, data, buffer);
        }

        void VulkanDevice::CreateTexture(const TextureDescriptor* desc, void* /*data*/, Texture* /*texture*/) const
        {
            SASSERT(desc);
            //auto internalState = static_cast<VulkanTexture>(texture->internalState);
        }

        void VulkanDevice::BindVertexBuffer(const GPUBuffer* buffer)
        {
            SASSERT(buffer);
            SASSERT(buffer->IsValid());
            SASSERT(buffer->IsBuffer());

            const VulkanBuffer* internalBuffer = static_cast<VulkanBuffer*>(buffer->internalState.get());
            SASSERT(internalBuffer);

            VkCommandBuffer& cmd = CommandBuffers.at(FrameIndex);

            VkDeviceSize offset = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, internalBuffer->GetHandle(), &offset);
        }

        void VulkanDevice::BindIndexBuffer(const GPUBuffer* buffer)
        {
            SASSERT(buffer);
            SASSERT(buffer->IsValid());
            SASSERT(buffer->IsBuffer());

            const VulkanBuffer* internalBuffer = static_cast<VulkanBuffer*>(buffer->internalState.get());
            SASSERT(internalBuffer);

            VkCommandBuffer& cmd = CommandBuffers.at(FrameIndex);

            VkDeviceSize offset = {0};
            vkCmdBindIndexBuffer(cmd, *internalBuffer->GetHandle(), offset, VK_INDEX_TYPE_UINT32);
        }

        void VulkanDevice::SetTopology(PrimitiveTopology topology)
        {
            VkCommandBuffer& cmd = CommandBuffers.at(FrameIndex);
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

        void VulkanDevice::Draw(const u32 count, const u32 offset)
        {
            SASSERT(count > 0);
            SASSERT(offset >= 0);

            VkCommandBuffer& cmd = CommandBuffers[FrameIndex];
            vkCmdDraw(cmd, count, 1, offset, 0);
        }

        void VulkanDevice::DrawIndexed(const u32 count, const u32 offset)
        {
            SASSERT(count > 0);
            SASSERT(offset >= 0);

            VkCommandBuffer& cmd = CommandBuffers.at(FrameIndex);
            vkCmdDrawIndexed(cmd, count, 1, offset, 0, 0);
        }

        void VulkanDevice::activateObject(const glm::mat4 &model, const glm::vec4 & /*color*/)
        {
            // glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(0, 0, -5));
            vkCmdPushConstants(
                CommandBuffers.at(FrameIndex),
                PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(glm::mat4), &model);
        }

        void VulkanDevice::activateCamera(const TCompCamera *camera)
        {
            // UpdateUniformBuffer();
            ConstantsCamera ubo;
            ubo.camera_view = camera->GetView();
            ubo.camera_projection = camera->GetProjection();
            ubo.camera_view_projection = camera->GetViewProjection();
            ubo.camera_projection[1][1] *= -1;

            memcpy(UniformBuffersMapped.at(FrameIndex), &ubo, sizeof(ubo));

            vkCmdBindDescriptorSets(
                CommandBuffers.at(FrameIndex),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PipelineLayout,
                0, 1,
                &DescriptorSets.at(FrameIndex),
                0, nullptr);
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

        void VulkanDevice::CreateDescriptorSetLayout()
        {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
            descriptorSetLayoutBinding.binding = 0;
            descriptorSetLayoutBinding.descriptorCount = 1;
            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            descriptorSetLayoutCreateInfo.bindingCount = 1;
            descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

            if (vkCreateDescriptorSetLayout(Handle, &descriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        void VulkanDevice::CreateGraphicsPipeline()
        {
            STRACE("\tReading compiled shaders ...");
            auto VertexShader = ReadFile("../../data/shaders/triangle.vert.spv");
            SASSERT(!VertexShader.empty());
            auto FragmentShader = ReadFile("../../data/shaders/triangle.frag.spv");
            SASSERT(!FragmentShader.empty());

            STRACE("\tCreating Shader Modules ...");
            VkShaderModule VertexShaderModule = CreateShaderModule(VertexShader);
            VkShaderModule FragmentShaderModule = CreateShaderModule(FragmentShader);
            STRACE("\tShader Modules created.");

            VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            VertexShaderStageCreateInfo.module = VertexShaderModule;
            VertexShaderStageCreateInfo.pName = "main";

            VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            FragmentShaderStageCreateInfo.module = FragmentShaderModule;
            FragmentShaderStageCreateInfo.pName = "main";

            VkPipelineShaderStageCreateInfo ShaderStages[] = {VertexShaderStageCreateInfo, FragmentShaderStageCreateInfo};

            std::vector<VkDynamicState> dynamicState = {
                VK_DYNAMIC_STATE_SCISSOR,
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                VK_DYNAMIC_STATE_LINE_WIDTH};

            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            dynamicStateCreateInfo.dynamicStateCount = static_cast<u32>(dynamicState.size());
            dynamicStateCreateInfo.pDynamicStates = dynamicState.data();

            VkVertexInputBindingDescription vertexBinding{};
            vertexBinding.binding = 0;
            vertexBinding.stride = sizeof(VulkanVertex);
            vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            VkPipelineVertexInputStateCreateInfo vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            vertexInputState.vertexBindingDescriptionCount = 1;
            vertexInputState.pVertexBindingDescriptions = &vertexBinding;
            vertexInputState.vertexAttributeDescriptionCount = static_cast<u32>(GetVertexDeclaration("PosColor")->size);
            vertexInputState.pVertexAttributeDescriptions = GetVertexDeclaration("PosColor")->layout;

            VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = 640.0f; //(float)Extent.width;
            viewport.height = 480.0f; //(float)Extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {640, 480}; //Extent;

            VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;

            VkPushConstantRange pushConstantRange = {};
            pushConstantRange.size = sizeof(glm::mat4);
            pushConstantRange.offset = 0;
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(Handle, &pipelineLayoutCreateInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create pipeline layout.");
            }

            STRACE("\tCreating Render Pass ...");
            CreateRenderPass();

            STRACE("\tCreating Graphics pipeline ...");
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
            graphicsPipelineCreateInfo.stageCount = 2;
            graphicsPipelineCreateInfo.pStages = ShaderStages;
            graphicsPipelineCreateInfo.layout = PipelineLayout;
            graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
            graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
            graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
            graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
            graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
            graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
            graphicsPipelineCreateInfo.pViewportState = &viewportState;
            graphicsPipelineCreateInfo.renderPass = RenderPass;
            graphicsPipelineCreateInfo.subpass = 0;
            graphicsPipelineCreateInfo.pDepthStencilState = nullptr;

            if (vkCreateGraphicsPipelines(Handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &Pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create graphics pipeline.");
            }

            STRACE("\tGraphics pipeline created.");

            STRACE("\tCleaning shade modules used ...");
            vkDestroyShaderModule(Handle, VertexShaderModule, nullptr);
            vkDestroyShaderModule(Handle, FragmentShaderModule, nullptr);
        }

        void VulkanDevice::CreateRenderPass()
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;// SurfaceFormat.format;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkSubpassDependency subpassDependency = {};
            subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency.dstSubpass = 0;
            subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency.srcAccessMask = 0;
            subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &subpassDependency;

            if (vkCreateRenderPass(Handle, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create render pass.");
            }
        }

        void VulkanDevice::CreateFramebuffers()
        {
            /*
            STRACE("\tCreating framebuffers ...");
            SwapchainFramebuffers.resize(SwapchainImageViews.size());

            for (size_t i = 0; i < SwapchainImageViews.size(); i++)
            {
                VkImageView attachments[] = {
                    SwapchainImageViews[i]};

                VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                createInfo.attachmentCount = 1;
                createInfo.pAttachments = attachments;
                createInfo.renderPass = RenderPass;
                createInfo.width = 640; //Extent.width;
                createInfo.height = 480; //Extent.height;
                createInfo.layers = 1;

                if (vkCreateFramebuffer(Device, &createInfo, nullptr, &SwapchainFramebuffers.at(i)) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create framebuffer.");
                }
            }
            */
        }

        void VulkanDevice::CreateCommandPool()
        {
            STRACE("\tCreating Command Pool ...");
            VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            createInfo.queueFamilyIndex = GraphicsFamily;
            createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            if (vkCreateCommandPool(Handle, &createInfo, nullptr, &CommandPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create command pool!");
            }
        }

        void VulkanDevice::CreateCommandBuffer()
        {
            STRACE("\tAllocating Command Buffers ...");
            CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            allocInfo.commandPool = CommandPool;
            allocInfo.commandBufferCount = static_cast<u32>(CommandBuffers.size());
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            if (vkAllocateCommandBuffers(Handle, &allocInfo, CommandBuffers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allcoate command buffer!");
            }
        }

        void VulkanDevice::CreateUniformBuffer()
        {
            VkDeviceSize bufferSize = sizeof(ConstantsCamera);

            UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            UniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
            UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                CreateBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    UniformBuffers.at(i), UniformBufferMemory.at(i));

                vkMapMemory(Handle, UniformBufferMemory.at(i), 0, bufferSize, 0, &UniformBuffersMapped.at(i));
            }
        }

        void VulkanDevice::CreateDescriptorPools()
        {
            VkDescriptorPoolSize poolSize = {};
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT);

            VkDescriptorPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            createInfo.maxSets = static_cast<u32>(MAX_FRAMES_IN_FLIGHT);
            createInfo.poolSizeCount = 1;
            createInfo.pPoolSizes = &poolSize;

            if (vkCreateDescriptorPool(Handle, &createInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor pool.");
            }
        }

        void VulkanDevice::CreateDescriptorSets()
        {
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, DescriptorSetLayout);

            VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            allocateInfo.descriptorPool = DescriptorPool;
            allocateInfo.descriptorSetCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT);
            allocateInfo.pSetLayouts = layouts.data();

            DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
            if (vkAllocateDescriptorSets(Handle, &allocateInfo, DescriptorSets.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate descriptor sets.");
            }

            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = UniformBuffers.at(i);
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(ConstantsCamera);

                VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                descriptorWrite.dstSet = DescriptorSets.at(i);
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;

                vkUpdateDescriptorSets(Handle, 1, &descriptorWrite, 0, nullptr);
            }
        }

        void VulkanDevice::CreateSyncObjects()
        {
            STRACE("\tCreating Sync objects ...");

            ImageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
            RenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
            InFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

            VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

            VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                if (vkCreateSemaphore(Handle, &semaphoreCreateInfo, nullptr, &ImageAvailableSemaphore.at(i)) ||
                    vkCreateSemaphore(Handle, &semaphoreCreateInfo, nullptr, &RenderFinishedSemaphore.at(i)) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create semaphore!");
                }

                if (vkCreateFence(Handle, &fenceCreateInfo, nullptr, &InFlightFence.at(i)) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create fence!");
                }
            }
        }

        void VulkanDevice::UpdateUniformBuffer()
        {
            ConstantsCamera ubo;
            ubo.camera_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            //ubo.camera_projection = glm::perspective(glm::radians(45.0f), static_cast<f32>(Extent.width / Extent.height), 0.1f, 100.0f);
            ubo.camera_view_projection = ubo.camera_projection * ubo.camera_view;
            ubo.camera_projection[1][1] *= -1;

            memcpy(UniformBuffersMapped.at(FrameIndex), &ubo, sizeof(ubo));
        }

        bool VulkanDevice::beginFrame()
        {
            /*
            if (vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, ImageAvailableSemaphore.at(FrameIndex), VK_NULL_HANDLE, &ImageIndex) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to acquire next image!");
            }

            if (vkWaitForFences(Device, 1, &InFlightFence.at(FrameIndex), VK_TRUE, UINT64_MAX) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to wait for fences!");
            }

            if (vkResetFences(Device, 1, &InFlightFence.at(FrameIndex)) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to reset fence!");
            }

            VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

            if (vkBeginCommandBuffer(CommandBuffers.at(FrameIndex), &cmdBeginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkViewport viewport{};
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = 640.0f; //(f32)Extent.width;
            viewport.height = 480.0f; //(f32)Extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vkCmdSetViewport(CommandBuffers.at(FrameIndex), 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.extent.width = 640; //Extent.width;
            scissor.extent.height = 480; //Extent.height;
            scissor.offset = {0, 0};

            vkCmdSetScissor(CommandBuffers.at(FrameIndex), 0, 1, &scissor);

            vkCmdSetLineWidth(CommandBuffers.at(FrameIndex), 1.0f);

            VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

            VkRenderPassBeginInfo renderpassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            renderpassBeginInfo.renderPass = RenderPass;
            renderpassBeginInfo.clearValueCount = 1;
            renderpassBeginInfo.pClearValues = &clearValue;
            renderpassBeginInfo.framebuffer = SwapchainFramebuffers[ImageIndex];
            renderpassBeginInfo.renderArea.extent = {640, 480}; //Extent;
            renderpassBeginInfo.renderArea.offset = {0, 0};

            vkCmdBeginRenderPass(CommandBuffers.at(FrameIndex), &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(CommandBuffers.at(FrameIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
            */
            return true;
        }

        void VulkanDevice::endFrame()
        {
            vkCmdEndRenderPass(CommandBuffers.at(FrameIndex));

            if (vkEndCommandBuffer(CommandBuffers.at(FrameIndex)) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to end recording command buffer");
            }

            Submit();

            FrameCount++;
            FrameIndex = (FrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        void VulkanDevice::Submit()
        {
            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &CommandBuffers.at(FrameIndex);
            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &RenderFinishedSemaphore.at(FrameIndex);
            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &ImageAvailableSemaphore.at(FrameIndex);
            submit.pWaitDstStageMask = &waitStages;

            if (vkQueueSubmit(GraphicsQueue, 1, &submit, InFlightFence.at(FrameIndex)) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to submit commands.");
            }

            /*
            VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
            presentInfo.pImageIndices = &ImageIndex;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &RenderFinishedSemaphore.at(FrameIndex);
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &Swapchain;

            if (vkQueuePresentKHR(GraphicsQueue, &presentInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to present");
            }
            */
        }

        VkShaderModule VulkanDevice::CreateShaderModule(const std::vector<char> &code)
        {
            VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const u32 *>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(Handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create shader module.");
            }
            return shaderModule;
        }

        void VulkanDevice::CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory)
        {
            VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.size = size;
            createInfo.usage = usageFlags;

            if (vkCreateBuffer(Handle, &createInfo, nullptr, &buffer) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create buffer ...");
            }

            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(Handle, buffer, &memoryRequirements);

            VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);
            allocateInfo.allocationSize = memoryRequirements.size;

            if (vkAllocateMemory(Handle, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate memory for buffer ...");
            }

            vkBindBufferMemory(Handle, buffer, bufferMemory, 0);
        }

        void VulkanDevice::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
        {
            VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            allocateInfo.commandBufferCount = 1;
            allocateInfo.commandPool = CommandPool;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VkCommandBuffer cmdBuffer;
            if (vkAllocateCommandBuffers(Handle, &allocateInfo, &cmdBuffer) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate command buffer.");
            }

            VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to start recording copy command buffer.");
            }

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;

            vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);

            vkEndCommandBuffer(cmdBuffer);

            VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmdBuffer;

            if (vkQueueSubmit(GraphicsQueue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to submit copy buffer commands.");
            }

            vkQueueWaitIdle(GraphicsQueue);

            vkFreeCommandBuffers(Handle, CommandPool, 1, &cmdBuffer);
        }

    } // Vk
} // Sogas
