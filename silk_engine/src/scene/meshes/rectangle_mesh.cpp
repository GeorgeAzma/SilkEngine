#include "rectangle_mesh.h"

RectangleMesh::RectangleMesh()
{
	vertices.resize(4);

	vertices[0].position = { 0, 0 };
	vertices[1].position = { 0, 1 };
	vertices[2].position = { 1, 1 };
	vertices[3].position = { 1, 0 };

	vertices[0].texture_coordinates = { 0, 1 };
	vertices[1].texture_coordinates = { 0, 0 };
	vertices[2].texture_coordinates = { 1, 0 };
	vertices[3].texture_coordinates = { 1, 1 };

	indices = { 2, 1, 0, 0, 3, 2 };
}
