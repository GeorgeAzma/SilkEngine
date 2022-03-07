#pragma once

#include <vulkan/vulkan.hpp>

struct SamplerProps
{
	vk::Filter min_filter = vk::Filter::eLinear;
	vk::Filter mag_filter = vk::Filter::eLinear;
	vk::SamplerAddressMode u_wrap = vk::SamplerAddressMode::eClampToEdge;
	vk::SamplerAddressMode v_wrap = vk::SamplerAddressMode::eClampToEdge;
	vk::SamplerAddressMode w_wrap = vk::SamplerAddressMode::eClampToEdge;
	bool anisotropy = false;
	bool linear_mipmap = true;
	uint32_t mip_levels = 1;
};

class Sampler
{
public:
	Sampler(const SamplerProps& props = {});
	~Sampler();

	operator const vk::Sampler& () const { return sampler; }

private:
	vk::Sampler sampler;
};