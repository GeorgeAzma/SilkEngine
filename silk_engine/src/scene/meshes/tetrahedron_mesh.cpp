#include "tetrahedron_mesh.h"

TetrahedronMesh::TetrahedronMesh()
{
	resizeVertices(12);
	resizeIndices(12);

	getVertex(0).position = { 0.815363f, -0.33287f, -0.47075f }; //A
	getVertex(1).position = { 0.0f, -0.3328f, 0.9415f }; //B
	getVertex(2).position = { -0.815363f, -0.33287f, -0.47075f }; //D

	getVertex(0).normal = { 0, -1, 0 };
	getVertex(1).normal = { 0, -1, 0 };
	getVertex(2).normal = { 0, -1, 0 };

	getVertex(0).texture_coordinate = { 0, 0 };
	getVertex(1).texture_coordinate = { 1, 0 };
	getVertex(2).texture_coordinate = { 0, 1 };

	//Tri-2
	getVertex(3).position = { 0.815363f, -0.33287f, -0.47075f }; //A
	getVertex(4).position = { -0.815363f, -0.33287f, -0.47075f }; //D
	getVertex(5).position = { 0.0f, 1.0f, 0.0f }; //C

	getVertex(3).normal = { 0, 0.70710678118f, -0.70710678118f };
	getVertex(4).normal = { 0, 0.70710678118f, -0.70710678118f };
	getVertex(5).normal = { 0, 0.70710678118f, -0.70710678118f };

	getVertex(3).texture_coordinate = { 0, 0 };
	getVertex(4).texture_coordinate = { 1, 0 };
	getVertex(5).texture_coordinate = { 0, 1 };

	//Tri-3
	getVertex(6).position = { 0.815363f, -0.33287f, -0.47075f }; //A
	getVertex(7).position = { 0.0f, 1.0f, 0.0f }; //C
	getVertex(8).position = { 0.0f, -0.3328f, 0.9415f }; //B

	getVertex(6).normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };
	getVertex(7).normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };
	getVertex(8).normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };

	getVertex(6).texture_coordinate = { 0, 0 };
	getVertex(7).texture_coordinate = { 1, 0 };
	getVertex(8).texture_coordinate = { 0, 1 };

	//Tri-4
	getVertex(9 ).position = { -0.815363f, -0.33287f, -0.47075f }; //D
	getVertex(10).position = { 0.0f, -0.3328f, 0.9415f }; //B
	getVertex(11).position = { 0.0f, 1.0f, 0.0f }; //C

	getVertex(9 ).normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };
	getVertex(10).normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };
	getVertex(11).normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };

	getVertex(9 ).texture_coordinate = { 0, 0 };
	getVertex(10).texture_coordinate = { 1, 0 };
	getVertex(11).texture_coordinate = { 0, 1 };

	getIndex(0) = 0; getIndex(1) = 1; getIndex(2) = 2;
	getIndex(3) = 3; getIndex(4) = 4; getIndex(5) = 5;
	getIndex(6) = 6; getIndex(7) = 7; getIndex(8) = 8;
	getIndex(9) = 9; getIndex(10) = 10; getIndex(11) = 11;
}