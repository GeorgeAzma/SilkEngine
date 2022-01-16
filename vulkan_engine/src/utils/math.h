#pragma once

namespace math
{
	static constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);

	static constexpr float quarter_pi = 0.78539816339f;
	static constexpr float half_pi = 1.57079632679f;
	static constexpr float pi = 3.14159265359f;
	static constexpr float two_pi = 6.28318530717f;

	static glm::vec3 position(const glm::mat4& mat)
	{
		return { mat[3][0], mat[3][1], mat[3][2] };
	}

	static glm::vec3 eulerToDirection(const glm::vec3& euler_angles)
	{
		float cosy = cos(euler_angles.y);
		return glm::vec3(cos(euler_angles.x) * cosy, sin(euler_angles.y), sin(euler_angles.x) * cosy);
	}
};