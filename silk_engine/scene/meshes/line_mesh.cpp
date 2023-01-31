#include "line_mesh.h"

LineMesh::LineMesh(const std::vector<vec2>& points, float width, uint32_t edge_resolution, bool tile_UVs)
{
	width *= 0.5f;
	resizeVertices(points.size() * 2);
	resizeIndices(points.size() * 6);

	size_t index = 0;
	for (size_t i = 0; i < points.size() - 1; ++i)
	{
		getIndex(index++) = i * 2 + 0;
		getIndex(index++) = i * 2 + 2;
		getIndex(index++) = i * 2 + 1;
		getIndex(index++) = i * 2 + 1;
		getIndex(index++) = i * 2 + 2;
		getIndex(index++) = i * 2 + 3;
	}
	//TODO: Generate smooth edges
	size_t vertex = 0;
	const auto& point = points[0];
	const auto& next_point = points[1];
	vec2 dir = math::directionTo(point, next_point);
	vec2 cross = math::cross2D(dir);
	vec2 offset = cross * width;
	getVertex(vertex).position = point + offset;
	getVertex(vertex++).texture_coordinate = { 1, 0 };
	getVertex(vertex).position = point - offset;
	getVertex(vertex++).texture_coordinate = { 0, 0 };

	for (size_t i = 1; i < points.size(); ++i)
	{
		const auto& prev_point = points[i - 1];
		const auto& point = points[i];
		vec2 dir = math::directionTo(prev_point, point);
		vec2 cross = math::cross2D(dir);
		vec2 offset = cross * width;
		getVertex(vertex).position = point + offset;
		getVertex(vertex++).texture_coordinate = { 1, 0 };
		getVertex(vertex).position = point - offset;
		getVertex(vertex++).texture_coordinate = { 0, 0 };
	}

	if (!tile_UVs)
	{
		float length_sq = 0.0f;
		std::vector<float> lengths_sq(points.size());
		lengths_sq[0] = 0.0f;
		for (size_t i = 1; i < points.size(); ++i)
		{
			length_sq += math::distance2(points[i - 1], points[i]);
			lengths_sq[i] = length_sq;
		}

		for (size_t i = 0; i < lengths_sq.size(); ++i)
		{
			float t = lengths_sq[i] / length_sq;
			getVertex(i + 0).texture_coordinate.y = t;
			getVertex(i + 1).texture_coordinate.y = t;
		}
	}
}
