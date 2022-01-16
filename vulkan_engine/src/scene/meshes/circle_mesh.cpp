#include "circle_mesh.h"

CircleMesh::CircleMesh(unsigned int resolution)
{
	VE_CORE_ASSERT(resolution >= 2, "Circle Resolution is too low");
	const size_t verticesCount = resolution + 1; // + 1 for Center point
	const size_t indicesSize = (resolution) * 3;
	vertices.resize(verticesCount);
	indices.resize(indicesSize);
	constexpr glm::vec3 center(0);
	vertices[0].position = center;
	size_t res = resolution - 1;

	for (size_t i = 0; i < resolution; ++i)
	{
		float d = (float)i / res * glm::pi<float>() * 2;
		vertices[i + 1].position = glm::vec3(std::cos(d), std::sin(d), 0);
	}

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		vertices[i].texture_coordinates = (vertices[i].position + glm::vec3(1, 1, 0)) * glm::vec3(0.5f, 0.5f, 0.0f);
		vertices[i].texture_coordinates.x = 1 - vertices[i].texture_coordinates.x;
	}

	size_t index = 0;
	//+ 2 to avoid duplicates, because indices[0] = center and if(i == 0)indices[i - 1] = center
	for (uint32_t i = 2; i < resolution + 2; ++i)
	{
		indices[index++] = i - 1;
		indices[index++] = i;
		indices[index++] = 0;
	}

	init();
}
