#pragma once

class Random 
{
public:
	using result_type = uint64_t;
	static inline result_type seed = 3773452183ULL;

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
	static vec3 Vec3()
	{
		float theta = Float() * math::two_pi<float>();
		float phi = acos(Float() * 2 - 1);

		float x = sin(phi) * cos(theta);
		float y = sin(phi) * sin(theta);
		float z = cos(phi);

		return vec3(x, y, z);
	}
	static vec2 Vec2()
	{
		float theta = Float() * math::two_pi<float>();

		return vec2(cos(theta), sin(theta));
	}
	static constexpr result_type max()
	{
		return std::numeric_limits<result_type>::max();
	}
	static constexpr result_type min()
	{
		return std::numeric_limits<result_type>::min();
	}
	result_type operator()()
	{
		return next();
	}

private:
	static result_type next()
	{
		result_type z = (seed += result_type(0x9E3779B97F4A7C15));
		z = (z ^ (z >> 30)) *    result_type(0xBF58476D1CE4E5B9);
		z = (z ^ (z >> 27)) *    result_type(0x94D049BB133111EB);
		return z ^ (z >> 31);
	}
};