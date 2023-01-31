#include "frustum.h"

Frustum::Frustum(const mat4& projection_view)
{
	calculatePlanes(projection_view);
}

void Frustum::calculatePlanes(const mat4& projection_view) 
{
	const mat4 projection_view_t = math::transpose(projection_view);
	planes[LEFT] = projection_view_t[3] + projection_view_t[0];
	planes[RIGHT] = projection_view_t[3] - projection_view_t[0];
	planes[BOTTOM] = projection_view_t[3] + projection_view_t[1];
	planes[TOP] = projection_view_t[3] - projection_view_t[1];
	planes[NEAR] = projection_view_t[3] + projection_view_t[2];
	planes[FAR] = projection_view_t[3] - projection_view_t[2];
}

bool Frustum::isBoxVisible(const vec3& min, const vec3& max) const
{
	for (size_t i = 0; i < COUNT; ++i)
		if (math::dot(planes[i], vec4(min.x, min.y, min.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(max.x, min.y, min.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(min.x, max.y, min.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(max.x, max.y, min.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(min.x, min.y, max.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(max.x, min.y, max.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(min.x, max.y, max.z, 1.0f)) < 0.0f &&
			math::dot(planes[i], vec4(max.x, max.y, max.z, 1.0f)) < 0.0f)
			return false;
	return true;
}