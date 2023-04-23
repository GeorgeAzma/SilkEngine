#pragma once

class Sampler
{
public:
	enum class MipmapMode
	{
		NONE = 0, // This is used for image class to determine if using mipmapping or not
		NEAREST = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		LINEAR = VK_SAMPLER_MIPMAP_MODE_LINEAR
	};

	struct Props
	{
		VkFilter min_filter = VK_FILTER_LINEAR;
		VkFilter mag_filter = VK_FILTER_LINEAR;
		VkSamplerAddressMode u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode w_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		MipmapMode mipmap_mode = MipmapMode::NONE;
		float anisotropy = 1.0f; // 0.0f is max anisotropy level available
	};

public:
	Sampler(const Props& props = {});
	~Sampler();

	operator const VkSampler& () const { return sampler; }

private:
	VkSampler sampler = nullptr;

public:
	static shared<Sampler> get(const Props& props)
	{
		if (auto layout = samplers.find(props); layout != samplers.end())
			return layout->second;
		return add(props);
	}
	static shared<Sampler> add(const Props& props)
	{
		return samplers.insert_or_assign(props, makeShared<Sampler>(props)).first->second;
	}
	static void destroy() { samplers.clear(); }

private:
	struct Hash
	{
		size_t operator()(const Props& props) const
		{
			size_t result = props.min_filter ^ (props.mag_filter << 1) ^ (props.u_wrap << 4) ^ (props.v_wrap << 7) ^ (props.v_wrap << 10) ^ (size_t(props.mipmap_mode) << 12);
			return result ^ *(const uint32_t*)&props.anisotropy;
		}
	};

	struct Equal
	{
		bool operator()(const Props& props, const Props& other) const
		{
			return props.min_filter == other.min_filter && props.mag_filter == other.mag_filter && 
				props.u_wrap == other.u_wrap && props.v_wrap == other.v_wrap && props.w_wrap == other.w_wrap && props.mipmap_mode == other.mipmap_mode && props.anisotropy == other.anisotropy;
		}
	};

	static inline std::unordered_map<Props, shared<Sampler>, Hash, Equal> samplers{};
};