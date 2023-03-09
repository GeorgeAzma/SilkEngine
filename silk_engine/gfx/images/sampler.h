#pragma once

class Sampler
{
public:
	struct Props
	{
		VkFilter min_filter = VK_FILTER_LINEAR;
		VkFilter mag_filter = VK_FILTER_LINEAR;
		VkSamplerAddressMode u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode w_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		float anisotropy = 1.0f; // 0.0f is max anisotropy level available
		bool linear_mipmap = true;
	};

public:
	Sampler(const Props& props = {});
	~Sampler();

	operator const VkSampler& () const { return sampler; }

private:
	VkSampler sampler = nullptr;
};