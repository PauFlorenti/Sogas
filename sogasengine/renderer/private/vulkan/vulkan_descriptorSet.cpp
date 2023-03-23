#include "vulkan/vulkan_descriptorSet.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_sampler.h"
#include "vulkan/vulkan_texture.h"

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

static void FillWriteDescriptorSets(
  VulkanDevice*                    InDevice,
  const VulkanDescriptorSetLayout* InDescriptorSetLayout,
  VkDescriptorSet                  InDescriptorSet,
  VkWriteDescriptorSet*            InWriteDescriptorSet,
  VkDescriptorBufferInfo*          InBufferInfo,
  VkDescriptorImageInfo*           InImageInfo,
  u32&                             OutResourcesCount,
  const ResourceHandle*            InResources,
  const SamplerHandle*             InSamplers,
  const u16*                       InBindings)
{
    auto default_sampler = InDevice->GetDefaultSampler();

    u32 resources_used = 0;
    for (u32 i = 0; i < OutResourcesCount; ++i)
    {
        u32 layout_binding_index = InBindings[i];

        const DescriptorBinding& binding = InDescriptorSetLayout->bindings[layout_binding_index];

        InWriteDescriptorSet[resources_used]                 = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        InWriteDescriptorSet[resources_used].dstSet          = InDescriptorSet;
        InWriteDescriptorSet[resources_used].dstBinding      = binding.start;
        InWriteDescriptorSet[resources_used].dstArrayElement = 0;
        InWriteDescriptorSet[resources_used].descriptorCount = 1;

        switch (binding.type)
        {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                InWriteDescriptorSet[resources_used].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                TextureHandle  texture_handle = {InResources[i]};
                VulkanTexture* texture        = InDevice->GetTextureResource(texture_handle);

                InImageInfo[resources_used].sampler = default_sampler->sampler;

                if (texture->sampler)
                {
                    InImageInfo[resources_used].sampler = texture->sampler;
                }

                if (InSamplers[i].index != INVALID_ID)
                {
                    VulkanSampler* sampler              = InDevice->GetSamplerResource(InSamplers[i]);
                    InImageInfo[resources_used].sampler = sampler->sampler;
                }

                InWriteDescriptorSet[resources_used].pImageInfo = &InImageInfo[resources_used];

                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                InWriteDescriptorSet[resources_used].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                TextureHandle  texture_handle = {InResources[i]};
                VulkanTexture* texture        = InDevice->GetTextureResource(texture_handle);

                InImageInfo[resources_used].sampler     = nullptr;
                InImageInfo[resources_used].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                InImageInfo[resources_used].imageView   = texture->image_view;

                InWriteDescriptorSet[resources_used].pImageInfo = &InImageInfo[resources_used];

                break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            {
                InWriteDescriptorSet[resources_used].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                BufferHandle  buffer_handle = {InResources[i]};
                VulkanBuffer* buffer        = InDevice->GetBufferResource(buffer_handle);

                //InWriteDescriptorSet[resources_used].descriptorType = buffer->

                InBufferInfo[resources_used].buffer = buffer->buffer;
                InBufferInfo[resources_used].offset = 0;
                InBufferInfo[resources_used].range  = buffer->size;

                InWriteDescriptorSet[resources_used].pBufferInfo = &InBufferInfo[resources_used];

                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                InWriteDescriptorSet[resources_used].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

                BufferHandle  buffer_handle = {InResources[i]};
                VulkanBuffer* buffer        = InDevice->GetBufferResource(buffer_handle);

                //InWriteDescriptorSet[resources_used].descriptorType = buffer->

                InBufferInfo[resources_used].buffer = buffer->buffer;
                InBufferInfo[resources_used].offset = 0;
                InBufferInfo[resources_used].range  = buffer->size;

                InWriteDescriptorSet[resources_used].pBufferInfo = &InBufferInfo[resources_used];

                break;
            }
            default:
            {
                SASSERT_MSG(false, "Resource type not supported.");
                break;
            }
        }

        ++resources_used;
    }

    OutResourcesCount = resources_used;
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
    FillWriteDescriptorSets(
      InDevice,
      descriptor_set_layout,
      descriptor_set->descriptorSet,
      descriptor_write,
      buffer_info,
      image_info,
      resources_count,
      InDescriptor.resources,
      InDescriptor.samplers,
      InDescriptor.bindings);

    for (u32 i = 0; i < InDescriptor.resources_count; ++i)
    {
        descriptor_set->resources[i] = InDescriptor.resources[i];
        descriptor_set->samplers[i]  = InDescriptor.samplers[i];
        descriptor_set->bindings[i]  = InDescriptor.bindings[i];
    }

    vkUpdateDescriptorSets(InDevice->Handle, resources_count, descriptor_write, 0, nullptr);

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

void VulkanDescriptorSet::BindDescriptor(VkCommandBuffer cmd) const
{
    vkCmdBindDescriptorSets(cmd, pipelineBindPoint, pipelineLayout, setNumber, 1, &descriptorSet, 0, nullptr);
}
} // namespace Vk
} // namespace Renderer
} // namespace Sogas
