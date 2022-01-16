#pragma once

class BitUtils
{
public:
	template<typename T = size_t>
	static size_t getEnabledFlagsCount(T flags) 
	{
		size_t enabled_count = 0;
		for (size_t i = 0; i < sizeof(flags) * 8; ++i)
		{
			enabled_count += ((flags & T(1 << i)) == (1 << i));
		}

		return enabled_count;
	}

};