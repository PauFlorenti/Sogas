#include "vulkan/vulkan_sampler.h"
#include "vulkan/vulkan_device.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

constexpr static VkSamplerAddressMode ConvertAddressMode(SamplerDescriptor::SamplerAddressMode InAddressMode)
{
    switch (InAddressMode)
    {
        default:
        case SamplerDescriptor::SamplerAddressMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerDescriptor::SamplerAddressMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerDescriptor::SamplerAddressMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        case SamplerDescriptor::SamplerAddressMode::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerDescriptor::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
}

constexpr static VkFilter ConvertSamplerFilter(SamplerDescriptor::SamplerFilter InFilter)
{
    switch (InFilter)
    {
        default:
        case SamplerDescriptor::SamplerFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerDescriptor::SamplerFilter::LINEAR:
            return VK_FILTER_LINEAR;
        case SamplerDescriptor::SamplerFilter::CUBIC:
            return VK_FILTER_CUBIC_EXT;
    }
}

constexpr static VkSamplerMipmapMode ConverSamplerMipmapMode(SamplerDescriptor::SamplerMipmapMode InMip)
{
    switch (InMip)
    {
        default:
        case SamplerDescriptor::SamplerMipmapMode::NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerDescriptor::SamplerMipmapMode::LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

SamplerHandle VulkanSampler::Create(VulkanDevice* InDevice, const SamplerDescriptor& InDescriptor)
{
    SamplerHandle handle = {InDevice->samplers.ObtainResource()};
    if (handle.index == INVALID_ID)
    {
        return handle;
    }

    VulkanSampler* sampler = InDevice->GetSamplerResource(handle);

    sampler->address_mode_u = ConvertAddressMode(InDescriptor.address_mode_u);
    sampler->address_mode_v = ConvertAddressMode(InDescriptor.address_mode_v);
    sampler->address_mode_w = ConvertAddressMode(InDescriptor.address_mode_w);
    sampler->min_filter     = ConvertSamplerFilter(InDescriptor.min_filter);
    sampler->mag_filter     = ConvertSamplerFilter(InDescriptor.mag_filter);
    sampler->mip_filter     = ConverSamplerMipmapMode(InDescriptor.mipmap_filter);

    VkSamplerCreateInfo info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    info.addressModeU            = sampler->address_mode_u;
    info.addressModeV            = sampler->address_mode_v;
    info.addressModeW            = sampler->address_mode_w;
    info.minFilter               = sampler->min_filter;
    info.magFilter               = sampler->mag_filter;
    info.mipmapMode              = sampler->mip_filter;
    info.anisotropyEnable        = VK_FALSE;
    info.compareEnable           = VK_FALSE;
    info.unnormalizedCoordinates = VK_FALSE;
    info.borderColor             = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    vkcheck(vkCreateSampler(InDevice->Handle, &info, nullptr, &sampler->sampler));

    // TODO set resource name!

    return handle;
}

} // namespace Vk
} // namespace Renderer
} // namespace Sogas