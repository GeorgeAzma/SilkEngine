#include "tetrahedron_mesh.h"

TetrahedronMesh::TetrahedronMesh()
{
	vertices.resize(12);

	vertices[0].position = { 0.815363f, -0.33287f, -0.47075f }; //A
	vertices[1].position = { 0.0f, -0.3328f, 0.9415f }; //B
	vertices[2].position = { -0.815363f, -0.33287f, -0.47075f }; //D

	vertices[0].normal = { 0, -1, 0 };
	vertices[1].normal = { 0, -1, 0 };
	vertices[2].normal = { 0, -1, 0 };

	vertices[0].texture_coordinate = { 0, 0 };
	vertices[1].texture_coordinate = { 1, 0 };
	vertices[2].texture_coordinate = { 0, 1 };

	//Tri-2
	vertices[3].position = { 0.815363f, -0.33287f, -0.47075f }; //A
	vertices[4].position = { -0.815363f, -0.33287f, -0.47075f }; //D
	vertices[5].position = { 0.0f, 1.0f, 0.0f }; //C

	vertices[3].normal = { 0, 0.70710678118f, -0.70710678118f };
	vertices[4].normal = { 0, 0.70710678118f, -0.70710678118f };
	vertices[5].normal = { 0, 0.70710678118f, -0.70710678118f };

	vertices[3].texture_coordinate = { 0, 0 };
	vertices[4].texture_coordinate = { 1, 0 };
	vertices[5].texture_coordinate = { 0, 1 };

	//Tri-3
	vertices[6].position = { 0.815363f, -0.33287f, -0.47075f }; //A
	vertices[7].position = { 0.0f, 1.0f, 0.0f }; //C
	vertices[8].position = { 0.0f, -0.3328f, 0.9415f }; //B

	vertices[6].normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };
	vertices[7].normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };
	vertices[8].normal = { 0.57735026919f, 0.57735026919f, 0.57735026919f };

	vertices[6].texture_coordinate = { 0, 0 };
	vertices[7].texture_coordinate = { 1, 0 };
	vertices[8].texture_coordinate = { 0, 1 };

	//Tri-4
	vertices[9 ].position = { -0.815363f, -0.33287f, -0.47075f }; //D
	vertices[10].position = { 0.0f, -0.3328f, 0.9415f }; //B
	vertices[11].position = { 0.0f, 1.0f, 0.0f }; //C

	vertices[9 ].normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };
	vertices[10].normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };
	vertices[11].normal = { -0.57735026919f, 0.57735026919f, 0.57735026919f };

	vertices[9 ].texture_coordinate = { 0, 0 };
	vertices[10].texture_coordinate = { 1, 0 };
	vertices[11].texture_coordinate = { 0, 1 };

	indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
}