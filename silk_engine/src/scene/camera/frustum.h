#pragma once

#include "scene/AABB.h"

class Frustum
{
private:

#undef NEAR
#undef FAR

	enum Planes
	{
		LEFT = 0,
		RIGHT,
		BOTTOM,
		TOP,
		NEAR,
		FAR,

		COUNT,
		COMBINATIONS = COUNT * (COUNT - 1) / 2
	};

public:
	Frustum() = default;
	Frustum(const glm::mat4& projection_view);
	void calculatePlanes(const glm::mat4& projection_view);
	bool isBoxVisible(const AABB3D& aabb) const;
	std::array<glm::vec4, COUNT> getPlanes() const { return planes; }

private:
	std::array<glm::vec4, COUNT> planes;
};