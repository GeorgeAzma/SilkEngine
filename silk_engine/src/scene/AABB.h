#pragma once

struct AABB
{
	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);

	bool intersects(const AABB& other) const
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
};