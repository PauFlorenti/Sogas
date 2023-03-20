#pragma once

#include "vulkan_types.h"

namespace Sogas
{
namespace Renderer
{
namespace Vk
{

class VulkanDevice;

class VulkanSampler
{
  public:
    static SamplerHandle Create(VulkanDevice* InDevice, const SamplerDescriptor& InDescriptor);

    VkSampler sampler;

    VkFilter            min_filter = VK_FILTER_NEAREST;
    VkFilter            mag_filter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    std::string name;
};

} // namespace Vk
} // namespace Renderer
} // namespace Sogas