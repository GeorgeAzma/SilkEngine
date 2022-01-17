#pragma once

class RNG 
{
public:
	static inline uint64_t m_seed = 3773452183ULL;

public:
	static double Float()
	{
		return (double)next() / max();
	}
	static uint64_t Uint()
	{
		return next();
	}
	static int64_t Int()
	{
		return next();
	}
	static bool Bool()
	{
		return next() >> 63ULL;
	}
	static glm::vec3 Vec3()
	{
		float theta = Float() * 6.283185307179586476925286766559f;
		float phi = std::acos(Float() * 2 - 1);

		float x = std::sin(phi) * std::cos(theta);
		float y = std::sin(phi) * std::sin(theta);
		float z = std::cos(phi);

		return glm::vec3(x, y, z);
	}
	static glm::vec2 Vec2()
	{
		float theta = Float() * 6.283185307179586476925286766559f;

		return glm::vec2(cos(theta), sin(theta));
	}
	static void seed(uint64_t seed)
	{
		RNG::m_seed = seed;
	}
	static constexpr const uint64_t max()
	{
		return std::numeric_limits<uint64_t>::max();
	}
	static constexpr const uint64_t min()
	{
		return std::numeric_limits<uint64_t>::min();
	}

private:
	static uint64_t next()
	{
		uint64_t z = (m_seed += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
};