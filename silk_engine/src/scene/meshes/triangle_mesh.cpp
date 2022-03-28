#include "triangle_mesh.h"

TriangleMesh::TriangleMesh()
{
	float h = sqrt(3.0f) * 0.25f;
	TriangleMesh(-0.5f, -h, 0.0f, h, 0.5f, -h);
}

TriangleMesh::TriangleMesh(float x1, float y1, float x2, float y2, float x3, float y3)
{
	vertices.resize(3);
	vertices[0].position = { x1, y1 };
	vertices[0].texture_coordinate = { 0.0f, 0.0f };

	vertices[1].position = { x2, y2 };
	vertices[1].texture_coordinate = { 0.5f, 1.0f };

	vertices[2].position = { x3, y3 };
	vertices[2].texture_coordinate = { 1.0f, 0.0f };

	indices = { 2, 1, 0 };
}
