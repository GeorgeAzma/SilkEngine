#include "sphere_mesh.h"

SphereMesh::SphereMesh(uint32_t resolution)
{
	SK_ASSERT(resolution >= 2, "Sphere resolution is too low");
	size_t vertices_size = resolution * resolution;
	uint32_t lines = resolution - 1;
	size_t indices_size = lines * lines * 6;
	resizeVertices(vertices_size * 6);
	resizeIndices(indices_size * 6);
	float d = 1.0f / lines;
	constexpr vec3 faces[6] =
	{
		vec3(1, 0, 0), vec3(-1, 0, 0), 
		vec3(0, 1, 0), vec3(0, -1, 0), 
		vec3(0, 0, 1), vec3(0, 0, -1)
	};

	size_t indx = 0;
	for (size_t face = 0; face < 6; ++face)
	{
		const vec3& facing = faces[face];
		vec3 axisA(facing.y, facing.z, facing.x);
		vec3 axisB = math::cross(facing, axisA);
		size_t offset = face * vertices_size;
		for (size_t y = 0; y < resolution; ++y)
		{
			float dy = y * d;
			for (size_t x = 0; x < resolution; ++x)
			{
				size_t i = y * resolution + x;
				float dx = x * d;
				vec3 point = facing + ((dx - 0.5f) * 2 * axisA + (dy - 0.5f) * 2 * axisB);
				point = vec3
				(
					point.x * std::sqrt(1.0f - ((point.y * point.y) + (point.z * point.z)) * 0.5f + ((point.y * point.y) * (point.z * point.z)) * 0.3333333f),
					point.y * std::sqrt(1.0f - ((point.z * point.z) + (point.x * point.x)) * 0.5f + ((point.z * point.z) * (point.x * point.x)) * 0.3333333f),
					point.z * std::sqrt(1.0f - ((point.x * point.x) + (point.y * point.y)) * 0.5f + ((point.x * point.x) * (point.y * point.y)) * 0.3333333f)
				);
				getVertex(offset + i).position = point;
				getVertex(offset + i).normal = point;
				switch (face)
				{
					case 0:
						getVertex(offset + i).texture_coordinate = vec2(1.0f - dx, 1.0f - dy);
						break;
					case 1:
						getVertex(offset + i).texture_coordinate = vec2(1.0f - dx, 1.0f - dy);
						break;
					case 2:
						getVertex(offset + i).texture_coordinate = vec2(1.0f - dx, 1.0f - dy);
						break;
					case 3:
						getVertex(offset + i).texture_coordinate = vec2(1.0f - dx, 1.0f - dy);
						break;
					case 4:
						getVertex(offset + x * resolution + y).texture_coordinate = vec2(1.0f - dx, dy);
						break;
					case 5:
						getVertex(offset + x * resolution + y).texture_coordinate = vec2(dx, 1.0f - dy);
						break;
				}
			}
		}

		for (size_t y = 0; y < resolution; ++y)
		{
			for (size_t x = 0; x < resolution; ++x)
			{
				size_t ind = x + y * resolution;
				float dx = (float)x / lines;
				float dy = (float)y / lines;
				size_t off = offset + ind;

				vec3 point = facing + ((dx - 0.5f) * 2 * axisA + (dy - 0.5f) * 2 * axisB);
				point = pointOnCubeToPointOnSphere(point);
				getVertex(off).position = point;
				getVertex(off).normal = point;

				if (x != resolution - 1 && y != resolution - 1)
				{
					getIndex(indx++) = off;
					getIndex(indx++) = off + resolution;
					getIndex(indx++) = off + 1;
					getIndex(indx++) = off + 1;
					getIndex(indx++) = off + resolution;
					getIndex(indx++) = off + resolution + 1;
				}
			}
		}
	}
}

vec3 SphereMesh::pointOnCubeToPointOnSphere(const vec3& p)
{
	return vec3
	(
		p.x * sqrt(1.0f - ((p.y * p.y) + (p.z * p.z)) * 0.5f + ((p.y * p.y) * (p.z * p.z)) * 0.333333333333333f),
		p.y * sqrt(1.0f - ((p.z * p.z) + (p.x * p.x)) * 0.5f + ((p.z * p.z) * (p.x * p.x)) * 0.333333333333333f),
		p.z * sqrt(1.0f - ((p.x * p.x) + (p.y * p.y)) * 0.5f + ((p.x * p.x) * (p.y * p.y)) * 0.333333333333333f)
	);
}