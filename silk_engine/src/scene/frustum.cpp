#include "frustum.h"

Frustum::Frustum(const glm::mat4& projection_view)
{
	calculatePlanes(projection_view);
}

void Frustum::calculatePlanes(glm::mat4 projection_view) 
{
	projection_view = glm::transpose(projection_view);
	planes[LEFT] = projection_view[3] + projection_view[0];
	planes[RIGHT] = projection_view[3] - projection_view[0];
	planes[BOTTOM] = projection_view[3] + projection_view[1];
	planes[TOP] = projection_view[3] - projection_view[1];
	planes[NEAR] = projection_view[3] + projection_view[2];
	planes[FAR] = projection_view[3] - projection_view[2];
}

bool Frustum::isBoxVisible(const glm::vec3& min, const glm::vec3& max) const
{
	for (size_t i = 0; i < COUNT; ++i)
		if (glm::dot(planes[i], glm::vec4(min.x, min.y, min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(max.x, min.y, min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(min.x, max.y, min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(max.x, max.y, min.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(min.x, min.y, max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(max.x, min.y, max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(min.x, max.y, max.z, 1.0f)) < 0.0f &&
			glm::dot(planes[i], glm::vec4(max.x, max.y, max.z, 1.0f)) < 0.0f)
			return false;
	return true;
}