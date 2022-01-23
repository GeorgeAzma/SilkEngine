#pragma once

struct SamplerProps
{
	std::vector<VkFilter> filters;
	std::vector<VkSamplerAddressMode> sampler_address_modes;
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