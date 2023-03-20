#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_pipeline.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

constexpr static VkDescriptorType ConvertDescriptorType(DescriptorType InType)
{
    switch (InType)
    {
        default:
        case DescriptorType::SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::COMBINED_IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SAMPLED_IMAGE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::STORAGE_IMAGE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UNIFORM_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::STORAGE_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::STORAGE_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::UNIFOR_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::STORAGE_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet  descriptorSet  = VK_NULL_HANDLE;
    device                          = nullptr;
}

DescriptorSetHandle VulkanDescriptorSet::Create(VulkanDevice* InDevice, const DescriptorSetDescriptor& InDescriptor)
{
    DescriptorSetHandle handle = {InDevice->descriptorSets.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanDescriptorSet*             descriptor_set        = InDevice->GetDescriptorSetResource(handle);
    const VulkanDescriptorSetLayout* descriptor_set_layout = InDevice->GetDescriptorSetLayoutResource(InDescriptor.layout);

    VkDescriptorSetAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocate_info.descriptorPool              = InDevice->descriptor_pool;
    allocate_info.pSetLayouts                 = &descriptor_set_layout->descriptor_set_layout;

    vkcheck(vkAllocateDescriptorSets(InDevice->Handle, &allocate_info, &descriptor_set->descriptorSet));

    u8* memory                      = static_cast<u8*>(InDevice->allocator->allocate((sizeof(ResourceHandle) + sizeof(SamplerHandle) + sizeof(u16)) * InDescriptor.resources_count, 1));
    descriptor_set->resources       = reinterpret_cast<ResourceHandle*>(memory);
    descriptor_set->samplers        = reinterpret_cast<SamplerHandle*>(memory + sizeof(ResourceHandle) * InDescriptor.resources_count);
    descriptor_set->bindings        = reinterpret_cast<u16*>(memory + (sizeof(ResourceHandle) + sizeof(SamplerHandle)) * InDescriptor.resources_count);
    descriptor_set->resources_count = InDescriptor.resources_count;
    descriptor_set->layout          = descriptor_set_layout;

    // Update
    VkWriteDescriptorSet   descriptor_write[8];
    VkDescriptorBufferInfo buffer_info[8];
    VkDescriptorImageInfo  image_info[8];

    VulkanSampler* default_sampler = InDevice->GetDefaultSampler();

    u32 resources_count = InDescriptor.resources_count;

    return handle;
}

DescriptorSetLayoutHandle VulkanDescriptorSet::Create(VulkanDevice* InDevice, const DescriptorSetLayoutDescriptor& InDescriptor)
{
    DescriptorSetLayoutHandle handle = {InDevice->descriptorSetLayouts.ObtainResource()};

    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanDescriptorSetLayout* descriptor_set_layout = InDevice->GetDescriptorSetLayoutResource(handle);

    descriptor_set_layout->bindings_count = static_cast<u16>(InDescriptor.bindings_count);
    u8* memory                            = static_cast<u8*>(InDevice->allocator->allocate((sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * InDescriptor.bindings_count, 1));
    descriptor_set_layout->bindings       = reinterpret_cast<DescriptorBinding*>(memory);
    descriptor_set_layout->binding        = reinterpret_cast<VkDescriptorSetLayoutBinding*>(memory + sizeof(DescriptorBinding) * InDescriptor.bindings_count);
    descriptor_set_layout->handle         = handle;
    descriptor_set_layout->set_index      = static_cast<u16>(InDescriptor.set_index);

    u32 used_bindings = 0;
    for (u32 i = 0; i < InDescriptor.bindings_count; ++i)
    {
        DescriptorBinding&                            binding       = descriptor_set_layout->bindings[i];
        const DescriptorSetLayoutDescriptor::Binding& input_binding = InDescriptor.bindings[i];
        binding.start                                               = input_binding.start == UINT16_MAX ? static_cast<u16>(i) : input_binding.start;
        binding.count                                               = 1;
        binding.type                                                = ConvertDescriptorType(input_binding.type);
        binding.name                                                = input_binding.name;

        VkDescriptorSetLayoutBinding vulkan_binding = descriptor_set_layout->binding[used_bindings];
        ++used_bindings;

        vulkan_binding.binding         = binding.start;
        vulkan_binding.descriptorType  = binding.type;
        vulkan_binding.descriptorType  = input_binding.type == DescriptorType::UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : vulkan_binding.descriptorType;
        vulkan_binding.descriptorCount = 1;

        vulkan_binding.stageFlags         = VK_SHADER_STAGE_ALL;
        vulkan_binding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount                    = used_bindings;
    info.pBindings                       = descriptor_set_layout->binding;

    vkcheck(vkCreateDescriptorSetLayout(InDevice->Handle, &info, nullptr, &descriptor_set_layout->descriptor_set_layout));

    return handle;
}

void VulkanDescriptorSet::Create(VulkanDevice*                                    InDevice,
                                 DescriptorSet*                                   InDescriptorSet,
                                 VkDescriptorSetLayout                            InDescriptorSetLayout,
                                 VkPipelineLayout                                 InPipelineLayout,
                                 const std::vector<VkDescriptorSetLayoutBinding>& InBindings,
                                 const u32                                        InSetNumber)
{
    SASSERT(InDescriptorSet);

    auto internalState               = std::make_shared<VulkanDescriptorSet>(InSetNumber);
    internalState->device            = InDevice;
    InDescriptorSet->internalState   = internalState;
    internalState->pipelineLayout    = InPipelineLayout;
    internalState->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    u32 numberBuffers  = 0;
    u32 numberSamplers = 0;
    for (const auto& binding : InBindings)
    {
        if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            numberBuffers++;
        }
        else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            numberSamplers++;
        }
    }

    // Create descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes;

    if (numberBuffers > 0)
    {
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numberBuffers});
    }

    if (numberSamplers > 0)
    {
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numberSamplers});
    }

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount              = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes                 = poolSizes.data();
    poolInfo.maxSets                    = 1;

    if (vkCreateDescriptorPool(InDevice->Handle, &poolInfo, nullptr, &InDevice->descriptor_pool) != VK_SUCCESS)
    {
        SERROR("Failed to create descriptor pool.");
        return;
    }

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool              = InDevice->descriptor_pool;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &InDescriptorSetLayout;

    // Allocate descriptor set
    VkResult res = vkAllocateDescriptorSets(InDevice->Handle, &allocInfo, &internalState->descriptorSet);
    if (res != VK_SUCCESS)
    {
        SERROR("Failed to allocate descriptor set.");
        return;
    }
}

void VulkanDescriptorSet::BindDescriptor(VkCommandBuffer cmd) const
{
    vkCmdBindDescriptorSets(cmd, pipelineBindPoint, pipelineLayout, setNumber, 1, &descriptorSet, 0, nullptr);
}
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
