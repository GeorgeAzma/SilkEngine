#pragma once

struct AABB3D
{
	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);

	bool intersects(const AABB3D& other) const
	{
		return min.x >= other.min.x 
			&& min.y >= other.min.y 
			&& min.z >= other.min.z
			&& min.x <= other.max.x
			&& min.y <= other.max.y
			&& min.z <= other.max.z;
	}

	bool intersects(const glm::vec3& point)
	{
		return point.x >= min.x 
			&& point.x <= max.x 
			&& point.y >= min.y 
			&& point.y <= max.y 
			&& point.z >= min.z 
			&& point.z <= max.z;
	}

	static AABB3D translate(const AABB3D& aabb, const glm::vec3& vec)
	{
		return AABB3D(aabb.min + vec, aabb.max + vec);
	}
};


struct AABB2D
{
	glm::vec2 min = glm::vec2(0);
	glm::vec2 max = glm::vec2(0);

	bool intersects(const AABB2D& other) const
	{
		return min.x >= other.min.x
			&& min.y >= other.min.y
			&& min.x <= other.max.x
			&& min.y <= other.max.y;
	}

	bool intersects(const glm::vec2& point)
	{
		return point.x >= min.x
			&& point.x <= max.x
			&& point.y >= min.y
			&& point.y <= max.y;
	}

	static AABB2D translate(const AABB2D& aabb, const glm::vec2& vec)
	{
		return AABB2D(aabb.min + vec, aabb.max + vec);
	}
};