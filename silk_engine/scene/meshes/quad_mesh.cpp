#include "quad_mesh.h"

QuadMesh::QuadMesh()
{
	resize(4, 6);

	getVertex(0).position = { -1, -1 };
	getVertex(1).position = { -1,  1 };
	getVertex(2).position = {  1,  1 };
	getVertex(3).position = {  1, -1 };

	getVertex(0).texture_coordinate = { 0, 0 };
	getVertex(1).texture_coordinate = { 0, 1 };
	getVertex(2).texture_coordinate = { 1, 1 };
	getVertex(3).texture_coordinate = { 1, 0 };

	getIndex(0) = 2; getIndex(1) = 1; getIndex(2) = 0;
	getIndex(3) = 0; getIndex(4) = 3; getIndex(5) = 2;
}
