#include "quad_mesh.h"

QuadMesh::QuadMesh()
{
	resizeVertices(4);
	resizeIndices(6);

	getVertex(0).position = { -0.5f, -0.5f };
	getVertex(1).position = { -0.5f,  0.5f };
	getVertex(2).position = {  0.5f,  0.5f };
	getVertex(3).position = {  0.5f, -0.5f };

	getVertex(0).texture_coordinate = { 0, 1 };
	getVertex(1).texture_coordinate = { 0, 0 };
	getVertex(2).texture_coordinate = { 1, 0 };
	getVertex(3).texture_coordinate = { 1, 1 };

	getIndex(0) = 2; getIndex(1) = 1; getIndex(2) = 0;
	getIndex(3) = 0; getIndex(4) = 3; getIndex(5) = 2;
}
