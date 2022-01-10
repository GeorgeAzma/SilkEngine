#pragma once

struct SamplerProps
{
	std::vector<VkFilter> filters;
	std::vector<VkSamplerAddressMode> sampler_address_modes;
	bool anisotropy = true;
	bool linearMipmap = true;
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