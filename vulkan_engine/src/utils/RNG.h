#pragma once

class RNG 
{
public:
	static inline uint64_t seed = 3773452183ULL;

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
		uint64_t z = (seed += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
};