#pragma once

struct SamplerProps
{
	VkFilter min_filter = VK_FILTER_LINEAR;
	VkFilter mag_filter = VK_FILTER_LINEAR;
	VkSamplerAddressMode u_wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode v_wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode w_wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	bool anisotropy = true;
	bool linear_mipmap = true;
	uint32_t mip_levels = 0;
};

class Sampler
{
public:
	Sampler(const SamplerProps& props = {});
	~Sampler();

	operator const VkSampler& () const { return sampler; }

private:
	VkSampler sampler;
};