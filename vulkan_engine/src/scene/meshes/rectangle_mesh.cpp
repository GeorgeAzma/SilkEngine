#include "rectangle_mesh.h"

RectangleMesh::RectangleMesh()
{
	vertices.resize(4);

	vertices[0].position = { 0.0f, 0.0f, 0.0f };
	vertices[1].position = { 1.0f, 0.0f, 0.0f };
	vertices[2].position = { 1.0f, 1.0f, 0.0f };
	vertices[3].position = { 0.0f, 1.0f, 0.0f };

	vertices[0].texture_coordinates = { 0.0f, 0.0f };
	vertices[1].texture_coordinates = { 1.0f, 0.0f };
	vertices[2].texture_coordinates = { 1.0f, 1.0f };
	vertices[3].texture_coordinates = { 0.0f, 1.0f };

	indices = { 0, 1, 2, 2, 3, 0 };

	init();
}
