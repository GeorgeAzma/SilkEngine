#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>

using glm::countof;

namespace math
{
	using namespace glm;

	inline constexpr vec3 FRONT(0.0f, 0.0f, 1.0f);
	inline constexpr vec3 UP(0.0f, 1.0f, 0.0f);

	inline vec3 position(const mat4& mat)
	{
		return { mat[3][0], mat[3][1], mat[3][2] };
	}

	inline vec2 position(const mat3& mat)
	{
		return { mat[2][0], mat[2][1] };
	}

	inline vec3 eulerToDirection(const vec3& euler_angles)
	{
		float cosx = cos(euler_angles.y);
		return vec3(cos(euler_angles.x) * cosx, sin(euler_angles.y), sin(euler_angles.x) * cosx);
	}

	template<typename T = float>
	inline T lerp(const T& a, const T& b, float k)
	{
		return a + (b - a) * k;
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	inline vec<2, T, Q> cross2D(const vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	inline vec<2, T, Q> cross2DCW(const vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	inline vec<2, T, Q> cross2DCCW(const vec<2, T, Q>& v)
	{
		return { -v.y, v.x };
	}

	template<length_t L, typename T, enum qualifier Q = qualifier::packed_highp>
	inline vec<L, T, Q> directionTo(const vec<L, T, Q>& p1, const vec<L, T, Q>& p2)
	{
		return normalize(p2 - p1);
	}

	template<typename T = float>
	inline T sigmoid(T x)
	{
		return T(1) / (T(1) + exp(-x));
	}

	template<typename T>
	inline T alignUp(const T& x, const T& alignment)
	{
		return (x % alignment) ? x + (alignment - (x % alignment)) : x;
	}

	template<typename T>
	inline T alignDown(const T& x, const T& alignment)
	{
		return x - (x % alignment);
	}

	template<typename T>
	inline T align(const T& x, const T& alignment)
	{
		return alignUp(x, alignment);
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	inline bool isPointInRectangle(const vec<2, T, Q>& p, const vec<4, T, Q> rectangle)
	{
		return p.x >= rectangle.x && p.y >= rectangle.y && p.x <= rectangle.x + rectangle.z && p.y < rectangle.y + rectangle.w;
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	inline bool isPointInCircle(const vec<2, T, Q>& p, const vec<3, T, Q> circle)
	{
		return distance2(p, { circle.x, circle.y }) <= circle.z * circle.z;
	}
};

using namespace glm;
#include <glm/fwd.hpp>