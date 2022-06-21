#pragma once

namespace math
{
	static constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
	static constexpr float QUARTER_PI = 0.78539816339f;
	static constexpr float HALF_PI = 1.57079632679f;
	static constexpr float PI = 3.14159265359f;
	static constexpr float TWO_PI = 6.28318530717f;

	static glm::vec3 position(const glm::mat4& mat)
	{
		return { mat[3][0], mat[3][1], mat[3][2] };
	}

	static glm::vec2 position(const glm::mat3& mat)
	{
		return { mat[2][0], mat[2][1] };
	}

	static glm::vec3 eulerToDirection(const glm::vec3& euler_angles)
	{
		float cosy = cos(euler_angles.y);
		return glm::vec3(cos(euler_angles.x) * cosy, sin(euler_angles.y), sin(euler_angles.x) * cosy);
	}

	template<typename T = size_t>
	static size_t getEnabledFlagsCount(T flags)
	{
		size_t enabled_count = 0;
		for (size_t i = 0; i < sizeof(flags) * 8; ++i)
			enabled_count += ((flags & T(1 << i)) == (1 << i));

		return enabled_count;
	}

	template<typename T = float>
	static T lerp(const T& a, const T& b, float k)
	{
		return a + (b - a) * (T)k;
	}

	template<typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	static glm::vec<2, T, Q> cross2D(const glm::vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	static glm::vec<2, T, Q> cross2DCW(const glm::vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	static glm::vec<2, T, Q> cross2DCCW(const glm::vec<2, T, Q>& v)
	{
		return { -v.y, v.x };
	}

	template<glm::length_t L, typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	static glm::vec<L, T, Q> directionTo(const glm::vec<L, T, Q>& p1, const glm::vec<L, T, Q>& p2)
	{
		return glm::normalize(p2 - p1);
	}

	template<typename T = float>
	static T sigmoid(T x)
	{
		return 1.0f / (1.0f + exp(-x));
	}
};