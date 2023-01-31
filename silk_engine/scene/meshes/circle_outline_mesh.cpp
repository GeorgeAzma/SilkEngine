#include "circle_outline_mesh.h"

CircleOutlineMesh::CircleOutlineMesh(uint resolution, float thickness)
{
	size_t vertices_count = resolution * 2;
	size_t indices_count = resolution * 6 + 6; //TODO:
	resizeVertices(vertices_count);
	resizeIndices(indices_count);

	size_t id = 0;

	for (size_t i = 0; i < resolution; ++i)
	{
		float d = (float)i / resolution * math::two_pi<float>();
		getVertex(id++).position = vec2(cos(d), sin(d));
		getVertex(id++).position = vec2(cos(d) * (1.0f - thickness), sin(d) * (1.0f - thickness));
	}

	for (size_t i = 0; i < vertices_count; ++i)
		getVertex(i).texture_coordinate = (getVertex(i).position + vec2(1)) * vec2(0.5f);

	id = 0;

	for (uint32_t i = 0; i < resolution; ++i)
	{
		getIndex(id++) = i * 2 + 2;
		getIndex(id++) = i * 2 + 1;
		getIndex(id++) = i * 2 + 0;

		getIndex(id++) = i * 2 + 1;
		getIndex(id++) = i * 2 + 2;
		getIndex(id++) = i * 2 + 3;
	}

	getIndex(id++) = 0;
	getIndex(id++) = 1;
	getIndex(id++) = resolution * 2 - 2;

	getIndex(id++) = 1;
	getIndex(id++) = resolution * 2 - 1;
	getIndex(id++) = resolution * 2 - 2;
}
