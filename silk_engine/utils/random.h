#pragma once

class Random
{
public:
	static inline uint64_t seed = 3773452183ULL;

public:

	template <typename T>
	static T get(uint64_t seed)
	{
		return getm<T>(seed);
	}
	
	template <typename T>
	static T get()
	{
		return getm<T>(seed);
	}

	size_t operator()()
	{
		return get<size_t>();
	}

private:
	static uint64_t next64(uint64_t& seed)
	{
		uint64_t z = (seed += 0x9E3779B97F4A7C15);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
		return z ^ (z >> 31);
	}
	static uint32_t next32(uint64_t& seed)
	{
		uint32_t z = (seed += 0X9E3779B9);
		z = (z ^ (z >> 15)) * 0X85EBCA6B;
		z = (z ^ (z >> 13)) * 0XC2B2AE35;
		return z ^ (z >> 16);
	}

	template <typename T>
	static T getm(uint64_t& seed)
	{
		if constexpr (sizeof(T) == sizeof(uint64_t))
			return next64(seed);
		return next32(seed);
	}
	template<> static float getm(uint64_t& seed) { return float(next32(seed)) / std::numeric_limits<uint32_t>::max(); }
	template<> static double getm(uint64_t& seed) { return double(next64(seed)) / std::numeric_limits<uint64_t>::max(); }
	template<> static bool getm(uint64_t& seed) { return next32(seed) & 1; }
	template<> static vec3 getm(uint64_t& seed)
	{
		float theta = get<float>(seed) * glm::two_pi<float>();
		float phi = acos(get<float>(seed) * 2 - 1);

		float x = sin(phi) * cos(theta);
		float y = sin(phi) * sin(theta);
		float z = cos(phi);

		return vec3(x, y, z);
	}
	template<> static vec2 getm(uint64_t& seed)
	{
		float theta = get<float>(seed) * glm::two_pi<float>();
		return vec2(cos(theta), sin(theta));
	}
};