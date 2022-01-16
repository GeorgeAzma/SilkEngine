#pragma once

namespace math
{
	static constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);

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