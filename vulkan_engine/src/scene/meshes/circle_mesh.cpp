#include "circle_mesh.h"

CircleMesh::CircleMesh(unsigned int resolution)
{
	VE_CORE_ASSERT(resolution >= 2, "Circle Resolution is too low");
	const size_t vertices_count = resolution + 1; // + 1 for Center point
	const size_t indices_count = resolution * 3;
	vertices.resize(vertices_count);
	indices.resize(indices_count);
	constexpr glm::vec3 center(0);
	vertices[0].position = center;

	for (size_t i = 0; i < resolution; ++i)
	{
		float d = (float)i / resolution * glm::two_pi<float>();
		vertices[i + 1].position = glm::vec3(cos(d), sin(d), 0);
	}

	for (size_t i = 0; i < vertices_count; ++i)
	{
		vertices[i].texture_coordinates = (vertices[i].position + glm::vec3(1, 1, 0)) * glm::vec3(0.5f, 0.5f, 0.0f);
		vertices[i].texture_coordinates.y = 1.0 - vertices[i].texture_coordinates.y;
	}

	size_t index = 0;
	for (uint32_t i = 1; i < resolution; ++i)
	{
		indices[index++] = 0;
		indices[index++] = i;
		indices[index++] = i + 1;
	}

	indices[index++] = 0;
	indices[index++] = resolution;
	indices[index++] = 1;

	init();
}
