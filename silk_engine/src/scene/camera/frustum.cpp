#include "frustum.h"

Frustum::Frustum(const glm::mat4& projection_view)
{
	calculatePlanes(projection_view);
}

void Frustum::calculatePlanes(const glm::mat4& projection_view) 
{
	const glm::mat4 projection_view_t = glm::transpose(projection_view);
	planes[LEFT] = projection_view_t[3] + projection_view_t[0];
	planes[RIGHT] = projection_view_t[3] - projection_view_t[0];
	planes[BOTTOM] = projection_view_t[3] + projection_view_t[1];
	planes[TOP] = projection_view_t[3] - projection_view_t[1];
	planes[NEAR] = projection_view_t[3] + projection_view_t[2];
	planes[FAR] = projection_view_t[3] - projection_view_t[2];
}

bool Frustum::isBoxVisible(const AABB& aabb) const
{
	for (size_t i = 0; i < COUNT; ++i)
		if (glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f)
			return false;
	return true;
}