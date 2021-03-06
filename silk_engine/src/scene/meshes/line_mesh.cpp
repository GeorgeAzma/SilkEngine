#include "line_mesh.h"

LineMesh::LineMesh(const std::vector<glm::vec2>& points, float width, uint32_t edge_resolution, bool tile_UVs)
{
	SK_ASSERT(points.size() >= 2, "Line must have at least 2 points");
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
	size_t odd = points.size() % 2 != 0;
	size_t pairs = points.size() - odd;
	for (size_t i = 0; i < pairs; i += 2)
	{
		const auto& point = points[i];
		const auto& next_point = points[i + 1];
		glm::vec2 dir = math::directionTo(point, next_point);
		glm::vec2 cross = math::cross2D(dir);
		glm::vec2 offset = cross * width;
		getVertex(vertex++).position = point + offset;
		getVertex(vertex++).position = point - offset;
		getVertex(vertex++).position = next_point + offset;
		getVertex(vertex++).position = next_point - offset;
	}
	if (odd)
	{
		const auto& point = points[points.size() - 1];
		const auto& prev_point = points[pairs - 1];
		glm::vec2 dir = math::directionTo(prev_point, point);
		glm::vec2 cross = math::cross2D(dir);
		glm::vec2 offset = cross * width;
		getVertex(vertex++).position = point + offset;
		getVertex(vertex++).position = point - offset;
	}


	if (tile_UVs)
	{
		for (size_t i = 0; i < pairs * 2; i += 4)
		{
			getVertex(i + 0).texture_coordinate = { 1.0f, 0.0f };
			getVertex(i + 1).texture_coordinate = { 0.0f, 0.0f };
			getVertex(i + 2).texture_coordinate = { 1.0f, 1.0f };
			getVertex(i + 3).texture_coordinate = { 0.0f, 1.0f };
		}
		if (odd)
		{
			getVertex(points.size() - 1).texture_coordinate = { 1.0f, 1.0f };
			getVertex(points.size() - 1).texture_coordinate = { 0.0f, 1.0f };
		}
	}
	else
	{
		float length_sq = 0.0f;
		std::vector<float> lengths_sq(points.size());
		lengths_sq[0] = 0.0f;
		for (size_t i = 1; i < points.size(); ++i)
		{
			length_sq += glm::distance2(points[i - 1], points[i]);
			lengths_sq[i] = length_sq;
		}

		for (size_t i = 0; i < lengths_sq.size(); ++i)
		{
			float t = lengths_sq[i] / length_sq;
			getVertex(i + 0).texture_coordinate = { 1.0f, t };
			getVertex(i + 1).texture_coordinate = { 0.0f, t };
		}
	}
}
