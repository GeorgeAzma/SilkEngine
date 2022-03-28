#include "quad_mesh.h"

QuadMesh::QuadMesh()
{
	vertices.resize(4);

	vertices[0].position = { -0.5f, -0.5f };
	vertices[1].position = { -0.5f,  0.5f };
	vertices[2].position = {  0.5f,  0.5f };
	vertices[3].position = {  0.5f, -0.5f };

	vertices[0].texture_coordinate = { 0, 1 };
	vertices[1].texture_coordinate = { 0, 0 };
	vertices[2].texture_coordinate = { 1, 0 };
	vertices[3].texture_coordinate = { 1, 1 };

	indices = { 2, 1, 0, 0, 3, 2 };
}
