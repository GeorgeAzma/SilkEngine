#include "sampler.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

Sampler::Sampler(const SamplerProps& props)
{
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = props.filters.size() >= 1 ? props.filters[0] : VK_FILTER_LINEAR;
	sampler_info.minFilter = props.filters.size() >= 2 ? props.filters[1] : sampler_info.magFilter;
	sampler_info.addressModeU = props.sampler_address_modes.size() >= 1 ? props.sampler_address_modes[0] : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = props.sampler_address_modes.size() >= 2 ? props.sampler_address_modes[1] : sampler_info.addressModeU;
	sampler_info.addressModeW = props.sampler_address_modes.size() >= 3 ? props.sampler_address_modes[2] : sampler_info.addressModeV;
	if (props.anisotropy)
	{
		sampler_info.anisotropyEnable = VK_TRUE;
		const VkPhysicalDeviceProperties& properties = Graphics::physical_device->getProperties();
		sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	}
	else
	{
		sampler_info.anisotropyEnable = VK_FALSE;
		sampler_info.maxAnisotropy = 1.0f;
	}
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = props.linearMipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = props.mip_levels;

	Graphics::vulkanAssert(vkCreateSampler(*Graphics::logical_device, &sampler_info, nullptr, &sampler));
}

Sampler::~Sampler()
{
	vkDestroySampler(*Graphics::logical_device, sampler, nullptr);
}
