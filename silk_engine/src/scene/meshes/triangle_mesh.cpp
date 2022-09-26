#include "triangle_mesh.h"

TriangleMesh::TriangleMesh() : TriangleMesh(-0.5f, -sqrt(3.0f) * 0.25f, 0.0f, sqrt(3.0f) * 0.25f, 0.5f, -sqrt(3.0f) * 0.25f)
{
}

TriangleMesh::TriangleMesh(float x1, float y1, float x2, float y2, float x3, float y3)
{
	resizeVertices(3);
	resizeIndices(3);

	getVertex(0).position = { x1, y1 };
	getVertex(0).texture_coordinate = { 0, 0 };

	getVertex(1).position = { x2, y2 };
	getVertex(1).texture_coordinate = { .5f, 1 };

	getVertex(2).position = { x3, y3 };
	getVertex(2).texture_coordinate = { 1, 0 };

	getIndex(0) = 2; getIndex(1) = 1; getIndex(2) = 0;
}
