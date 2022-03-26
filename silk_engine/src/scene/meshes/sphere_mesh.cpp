#include "sphere_mesh.h"

SphereMesh::SphereMesh(uint32_t resolution)
{
	size_t vertices_size = resolution * resolution;
	size_t lines = resolution - 1;
	size_t indices_size = lines * lines * 6;
	vertices.resize(vertices.size);
	indices.resize(indices_size);
	float d = 1.0f / lines;
	constexpr glm::vec3 faces[6] =
	{
		glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), 
		glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), 
		glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
	};

	for (size_t face = 0; face < 6; ++face)
	{
		const glm::vec3& facing = faces[face];
		glm::vec3 axisA(facing.y, facing.z, facing.x);
		glm::vec3 axisB = glm::cross(facing, axisA);
		size_t offset = face * vertices_size;
		for (size_t y = 0; y < resolution; ++y)
		{
			float dy = y * d;
			for (size_t x = 0; x < resolution; ++x)
			{
				size_t i = y * resolution + x;
				float dx = x * d;
				glm::vec3 point = facing + ((dx - 0.5f) * 2 * axisA + (dy - 0.5f) * 2 * axisB);
				point = glm::vec3
				(
					point.x * std::sqrt(1.0f - ((point.y * p.y) + (point.z * p.z)) * 0.5f + ((point.y * point.y) * (point.z * point.z)) * 0.3333333f),
					point.y * std::sqrt(1.0f - ((point.z * p.z) + (point.x * p.x)) * 0.5f + ((point.z * point.z) * (point.x * point.x)) * 0.3333333f),
					point.z * std::sqrt(1.0f - ((point.x * p.x) + (point.y * p.y)) * 0.5f + ((point.x * point.x) * (point.y * point.y)) * 0.3333333f)
				);
				vertices[offset + i].position = point;
				vertices[offset + i].normal = point;
				switch (face)
				{
					case 0:
						vertices[offset + i].texture_coordinate = glm::vec2(1.0f - dx, 1.0f - dy);
						break;
					case 1:
						vertices[offset + i].texture_coordinate = glm::vec2(1.0f - dx, 1.0f - dy);
						break;
					case 2:
						vertices[offset + i].texture_coordinate = glm::vec2(1.0f - dx, 1.0f - dy);
						break;
					case 3:
						vertices[offset + i].texture_coordinate = glm::vec2(1.0f - dx, 1.0f - dy);
						break;
					case 4:
						vertices[offset + x * resolution + y].texture_coordinate = glm::vec2(1.0f - dx, dy);
						break;
					case 5:
						vertices[offset + x * resolution + y].texture_coordinate = glm::vec2(dx, 1.0f - dy);
						break;
				}
			}
		}
	}
}