#include "circle_mesh.h"

CircleMesh::CircleMesh(unsigned int resolution)
{
	SK_ASSERT(resolution >= 2, "Circle Resolution is too low");
	const size_t vertices_count = resolution + 1; // + 1 for Center point
	const size_t indices_count = resolution * 3;
	resizeVertices(vertices_count);
	resizeIndices(indices_count);
	constexpr glm::vec2 center(0);
	getVertex(0).position = center;

	for (size_t i = 0; i < resolution; ++i)
	{
		float d = (float)i / resolution * glm::two_pi<float>();
		getVertex(i + 1).position = glm::vec2(cos(d), sin(d));
	}

	for (size_t i = 0; i < vertices_count; ++i)
	{
		getVertex(i).texture_coordinate = (getVertex(i).position + glm::vec2(1)) * glm::vec2(0.5f, 0.5f);
		getVertex(i).texture_coordinate.y = 1.0f - getVertex(i).texture_coordinate.y;
	}

	size_t index = 0;
	for (uint32_t i = 1; i < resolution; ++i)
	{
		getIndex(index++) = 0;
		getIndex(index++) = i;
		getIndex(index++) = i + 1;
	}

	getIndex(index++) = 0;
	getIndex(index++) = resolution;
	getIndex(index++) = 1;
}
 