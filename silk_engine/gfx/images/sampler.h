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
		bool linear_mipmap = true;
		float anisotropy = 1.0f; // 0.0f is max anisotropy level available
	};

public:
	Sampler(const Props& props = {});
	~Sampler();

	operator const VkSampler& () const { return sampler; }

private:
	VkSampler sampler = nullptr;

public:
	static shared<Sampler> get(const Props& props);
	static shared<Sampler> add(const Props& props);
	static void destroy() { samplers.clear(); }

private:
	struct Hash
	{
		size_t operator()(const Props& props) const
		{
			size_t result = props.min_filter ^ (props.mag_filter << 1) ^ (props.u_wrap << 4) ^ (props.v_wrap << 7) ^ (props.v_wrap << 10) ^ (props.linear_mipmap << 11);
			return result ^ *(const uint32_t*)&props.anisotropy;
		}
	};

	struct Equal
	{
		bool operator()(const Props& props, const Props& other) const
		{
			return props.min_filter == other.min_filter && props.mag_filter == other.mag_filter && 
				props.u_wrap == other.u_wrap && props.v_wrap == other.v_wrap && props.w_wrap == other.w_wrap && props.linear_mipmap == other.linear_mipmap && props.anisotropy == other.anisotropy;
		}
	};

	static inline std::unordered_map<Props, shared<Sampler>, Hash, Equal> samplers{};
};