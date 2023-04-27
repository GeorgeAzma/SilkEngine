#include "circle_mesh.h"

CircleMesh::CircleMesh(uint resolution)
{
	size_t vertices_count = resolution + 1; // + 1 for Center point
	size_t indices_count = resolution * 3;
	resize(vertices_count, indices_count);
	constexpr vec2 center(0);
	constexpr size_t center_index = 0;
	getVertex(0).position = center;

	for (size_t i = 0; i < resolution; ++i)
	{
		float d = (float)i / (resolution) * math::two_pi<float>();
		getVertex(i + 1).position = vec2(cos(d), sin(d));
	}

	for (size_t i = 0; i < vertices_count; ++i)
		getVertex(i).texture_coordinate = (getVertex(i).position + vec2(1)) * vec2(0.5f);

	size_t index = 0;
	for (uint32_t i = 1; i < resolution; ++i)
	{
		getIndex(index++) = center_index;
		getIndex(index++) = i;
		getIndex(index++) = i + 1;
	}

	getIndex(index++) = center_index;
	getIndex(index++) = resolution;
	getIndex(index++) = 1;
}
 