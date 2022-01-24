#include "sampler.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

Sampler::Sampler(const SamplerProps& props)
{
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = props.min_filter;
	sampler_info.minFilter = props.mag_filter;
	sampler_info.addressModeU = props.u_wrap;
	sampler_info.addressModeV = props.v_wrap;
	sampler_info.addressModeW = props.w_wrap;
	if (props.anisotropy)
	{
		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = Graphics::physical_device->getProperties().limits.maxSamplerAnisotropy;
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
	sampler_info.mipmapMode = props.linear_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = props.mip_levels;

	Graphics::vulkanAssert(vkCreateSampler(*Graphics::logical_device, &sampler_info, nullptr, &sampler));
}

Sampler::~Sampler()
{
	vkDestroySampler(*Graphics::logical_device, sampler, nullptr);
}
