#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::countof;

namespace math
{
	using namespace glm;

	static inline constexpr vec3 FRONT(0.0f, 0.0f, 1.0f);
	static inline constexpr vec3 UP(0.0f, 1.0f, 0.0f);

	static vec3 position(const mat4& mat)
	{
		return { mat[3][0], mat[3][1], mat[3][2] };
	}

	static vec2 position(const mat3& mat)
	{
		return { mat[2][0], mat[2][1] };
	}

	static vec3 eulerToDirection(const vec3& euler_angles)
	{
		float cosx = cos(euler_angles.y);
		return vec3(cos(euler_angles.x) * cosx, sin(euler_angles.y), sin(euler_angles.x) * cosx);
	}

	template<typename T = float>
	static T lerp(const T& a, const T& b, float k)
	{
		return a + (b - a) * k;
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static vec<2, T, Q> cross2D(const vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static vec<2, T, Q> cross2DCW(const vec<2, T, Q>& v)
	{
		return { v.y, -v.x };
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static vec<2, T, Q> cross2DCCW(const vec<2, T, Q>& v)
	{
		return { -v.y, v.x };
	}

	template<length_t L, typename T, enum qualifier Q = qualifier::packed_highp>
	static vec<L, T, Q> directionTo(const vec<L, T, Q>& p1, const vec<L, T, Q>& p2)
	{
		return normalize(p2 - p1);
	}

	template<typename T = float>
	static T sigmoid(T x)
	{
		return T(1) / (T(1) + exp(-x));
	}

	template<typename T>
	static T alignUp(const T& x, const T& alignment)
	{
		return (x % alignment) ? x + (alignment - (x % alignment)) : x;
	}

	template<typename T>
	static T alignDown(const T& x, const T& alignment)
	{
		return x - (x % alignment);
	}

	template<typename T>
	static T align(const T& x, const T& alignment)
	{
		return alignUp(x, alignment);
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static bool isPointInRectangle(const vec<2, T, Q>& p, const vec<4, T, Q>& rectangle)
	{
		return p.x >= rectangle.x && p.y >= rectangle.y && p.x <= rectangle.x + rectangle.z && p.y <= rectangle.y + rectangle.w;
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static bool isPointInCircle(const vec<2, T, Q>& p, const vec<3, T, Q>& circle)
	{
		return distance2(p, { circle.x, circle.y }) <= circle.z * circle.z;
	}

	template<typename T, enum qualifier Q = qualifier::packed_highp>
	static bool doRectanglesIntersect(const vec<4, T, Q>& rect1, const vec<4, T, Q>& rect2)
	{
		return rect1.x <= rect2.x + rect2.z && rect1.x + rect1.z >= rect2.x &&
			   rect1.y <= rect2.y + rect2.w && rect1.y + rect1.w >= rect2.y;
	}
};

using namespace glm;
#include <glm/fwd.hpp>