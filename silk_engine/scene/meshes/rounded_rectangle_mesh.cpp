#include "rounded_rectangle_mesh.h"

RoundedRectangleMesh::RoundedRectangleMesh(uint resolution, float roundness_x, float roundness_y)
{
	size_t vertex_count = resolution * 4 + 1;
	size_t index_count = ((resolution - 1) * 3) * 4 + 3 * 4;
	resize(vertex_count, index_count);
	uint res = resolution - 1;

	constexpr vec3 center(0.5f, 0.5f, 0.0f);
	constexpr size_t center_index = 0;

	// Center
	size_t id = 0;
	getVertex(id++).position = center;

	// Corners
	for (uint i = 0; i < resolution; ++i)
	{
		float d = float(i) / res * math::half_pi<float>();
		float cd = cos(d) * roundness_x;
		float sd = sin(d) * roundness_y;
		getVertex(id++).position = { cd + 1.0f, sd + 1.0f };
		getVertex(id++).position = { -cd, sd + 1.0f };
		getVertex(id++).position = { cd + 1.0f, -sd };
		getVertex(id++).position = { -cd, -sd };
	}

	id = 0;
	bool reverse_indices[4] = { false, true, true, false };
	for (uint32_t i = 0; i < resolution - 1; ++i)
	{
		for (uint32_t j = 0; j < 4; ++j)
		{
			uint32_t indices[2] = { i * 4 + j + 1, i * 4 + 4 + j + 1 };
			getIndex(id++) = indices[!reverse_indices[j] ? 0 : 1];
			getIndex(id++) = indices[!reverse_indices[j] ? 1 : 0];
			getIndex(id++) = center_index;
		}
	}

	getIndex(id++) = 3;
	getIndex(id++) = 1;
	getIndex(id++) = center_index;

	getIndex(id++) = 4 + 4 * (resolution - 1);
	getIndex(id++) = 3 + 4 * (resolution - 1);
	getIndex(id++) = center_index;

	getIndex(id++) = 2;
	getIndex(id++) = 4;
	getIndex(id++) = center_index;

	getIndex(id++) = 1 + 4 * (resolution - 1);
	getIndex(id++) = 2 + 4 * (resolution - 1);
	getIndex(id++) = center_index;

	//Gap Filler Triangle Indices
	//getIndex(id++) = resolution;
	//getIndex(id++) = resolution * 2;
	//getIndex(id++) = center_index;
	//
	//getIndex(id++) = center_index; //Center
	//getIndex(id++) = resolution * 2 + 1; //Big
	//getIndex(id++) = 1; //Small
	//
	//getIndex(id++) = center_index;
	//getIndex(id++) = resolution * 4;
	//getIndex(id++) = resolution * 3;
	//
	//getIndex(id++) = center_index;
	//getIndex(id++) = resolution + 1;
	//getIndex(id++) = resolution * 3 + 1;

	for (size_t i = 0; i < vertex_count; ++i)
	{
		vec2& pos = getVertex(i).position;
		pos += vec2{ roundness_x, roundness_y };
		pos /= vec2{ 1.0f + roundness_x * 2.0f, 1.0f + roundness_y * 2.0f };
		getVertex(i).uv = pos;
	}
}

RoundedRectangleMesh::RoundedRectangleMesh(uint resolution, float roundness)
	: RoundedRectangleMesh(resolution, roundness, roundness)
{
}
