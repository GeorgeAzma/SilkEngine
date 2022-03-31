#include "sampler.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

Sampler::Sampler(const SamplerProps& props)
{
	VkSamplerCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	ci.magFilter = props.min_filter;
	ci.minFilter = props.mag_filter;
	ci.addressModeU = props.u_wrap;
	ci.addressModeV = props.v_wrap;
	ci.addressModeW = props.w_wrap;
	if (props.anisotropy)
	{
		ci.anisotropyEnable = VK_TRUE;
		ci.maxAnisotropy = Graphics::physical_device->getProperties().limits.maxSamplerAnisotropy;
	}
	else
	{
		ci.anisotropyEnable = VK_FALSE;
		ci.maxAnisotropy = 1.0f;
	}
	ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	ci.unnormalizedCoordinates = VK_FALSE;
	ci.compareEnable = VK_FALSE;
	ci.compareOp = VK_COMPARE_OP_ALWAYS;
	ci.mipmapMode = props.linear_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	ci.mipLodBias = 0.0f;
	ci.minLod = 0.0f;
	ci.maxLod = props.mip_levels;
	sampler = Graphics::logical_device->createSampler(ci);
}

Sampler::~Sampler()
{
	Graphics::logical_device->destroySampler(sampler);
}
