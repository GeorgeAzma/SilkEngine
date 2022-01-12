#pragma once

namespace math
{
	static constexpr glm::vec3 UP(0.0f, 0.0f, 1.0f);

	static glm::vec3 position(const glm::mat4& mat)
	{
		return { mat[3][0], mat[3][1], mat[3][2] };
	}
};