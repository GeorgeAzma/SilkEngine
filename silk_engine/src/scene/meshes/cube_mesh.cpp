#include "cube_mesh.h"

CubeMesh::CubeMesh()
{
	//UNTESTED: Most likely false
	vertices.resize(8);

	vertices[0 ].position = { 0, 0, 0 };
	vertices[1 ].position = { 0, 0, 1 };
	vertices[2 ].position = { 0, 1, 1 };
	vertices[3 ].position = { 0, 0, 0 };
	vertices[4 ].position = { 0, 1, 1 };
	vertices[5 ].position = { 0, 1, 0 };
	vertices[6 ].position = { 1, 1, 0 };
	vertices[7 ].position = { 0, 0, 0 };
	vertices[8 ].position = { 0, 1, 0 };
	vertices[9 ].position = { 1, 0, 1 };
	vertices[10].position = { 0, 0, 0 };
	vertices[11].position = { 1, 0, 0 };
	vertices[12].position = { 1, 1, 0 };
	vertices[13].position = { 1, 0, 0 };
	vertices[14].position = { 0, 0, 0 };
	vertices[15].position = { 1, 0, 1 };
	vertices[16].position = { 0, 0, 1 };
	vertices[17].position = { 0, 0, 0 };
	vertices[18].position = { 0, 1, 1 };
	vertices[19].position = { 0, 0, 1 };
	vertices[20].position = { 1, 0, 1 };
	vertices[21].position = { 1, 1, 1 };
	vertices[22].position = { 1, 0, 0 };
	vertices[23].position = { 1, 1, 0 };
	vertices[24].position = { 1, 0, 0 };
	vertices[25].position = { 1, 1, 1 };
	vertices[26].position = { 1, 0, 1 };
	vertices[27].position = { 1, 1, 1 };
	vertices[28].position = { 1, 1, 0 };
	vertices[29].position = { 0, 1, 0 };
	vertices[30].position = { 1, 1, 1 };
	vertices[31].position = { 0, 1, 0 };
	vertices[32].position = { 0, 1, 1 };
	vertices[33].position = { 1, 1, 1 };
	vertices[34].position = { 0, 1, 1 };
	vertices[35].position = { 1, 0, 1 };

	vertices[0 ].texture_coordinate = { 0, 1 };
	vertices[1 ].texture_coordinate = { 0, 0 };
	vertices[2 ].texture_coordinate = { 1, 0 };
	vertices[3 ].texture_coordinate = { 1, 1 };
	vertices[4 ].texture_coordinate = { 0, 1 };
	vertices[5 ].texture_coordinate = { 0, 0 };
	vertices[6 ].texture_coordinate = { 1, 0 };
	vertices[7 ].texture_coordinate = { 1, 1 };
	vertices[8 ].texture_coordinate = { 0, 1 };
	vertices[9 ].texture_coordinate = { 0, 0 };
	vertices[10].texture_coordinate = { 1, 0 };
	vertices[11].texture_coordinate = { 1, 1 };
	vertices[12].texture_coordinate = { 0, 1 };
	vertices[13].texture_coordinate = { 0, 0 };
	vertices[14].texture_coordinate = { 1, 0 };
	vertices[15].texture_coordinate = { 1, 1 };
	vertices[16].texture_coordinate = { 0, 1 };
	vertices[17].texture_coordinate = { 0, 0 };
	vertices[18].texture_coordinate = { 1, 0 };
	vertices[19].texture_coordinate = { 1, 1 };
	vertices[20].texture_coordinate = { 0, 1 };
	vertices[21].texture_coordinate = { 0, 0 };
	vertices[22].texture_coordinate = { 1, 0 };
	vertices[23].texture_coordinate = { 1, 1 };
	vertices[24].texture_coordinate = { 0, 1 };
	vertices[25].texture_coordinate = { 0, 0 };
	vertices[26].texture_coordinate = { 1, 0 };
	vertices[27].texture_coordinate = { 1, 1 };
	vertices[28].texture_coordinate = { 0, 1 };
	vertices[29].texture_coordinate = { 0, 0 };
	vertices[30].texture_coordinate = { 1, 0 };
	vertices[31].texture_coordinate = { 1, 1 };
	vertices[32].texture_coordinate = { 0, 1 };
	vertices[33].texture_coordinate = { 0, 0 };
	vertices[34].texture_coordinate = { 1, 0 };
	vertices[35].texture_coordinate = { 1, 1 };

	vertices[0 ].normal = { -1, 0, 0 };
	vertices[1 ].normal = { -1, 0, 0 };
	vertices[2 ].normal = { -1, 0, 0 };
	vertices[3 ].normal = { -1, 0, 0 };
	vertices[4 ].normal = { -1, 0, 0 };
	vertices[5 ].normal = { -1, 0, 0 };
	vertices[6 ].normal = { 0, 0, 1 };
	vertices[7 ].normal = { 0, 0, 1 };
	vertices[8 ].normal = { 0, 0, 1 };
	vertices[9 ].normal = { 0, -1, 0 };
	vertices[10].normal = { 0, -1, 0 };
	vertices[11].normal = { 0, -1, 0 };
	vertices[12].normal = { 0, 0, 1 };
	vertices[13].normal = { 0, 0, 1 };
	vertices[14].normal = { 0, 0, 1 };
	vertices[15].normal = { 0, -1, 0 };
	vertices[16].normal = { 0, -1, 0 };
	vertices[17].normal = { 0, -1, 0 };
	vertices[18].normal = { 0, 1, 0 };
	vertices[19].normal = { 0, 1, 0 };
	vertices[20].normal = { 0, 1, 0 };
	vertices[21].normal = { 1, 0, 0 };
	vertices[22].normal = { 1, 0, 0 };
	vertices[23].normal = { 1, 0, 0 };
	vertices[24].normal = { 1, 0, 0 };
	vertices[25].normal = { 1, 0, 0 };
	vertices[26].normal = { 1, 0, 0 };
	vertices[27].normal = { 0, 1, 0 };
	vertices[28].normal = { 0, 1, 0 };
	vertices[29].normal = { 0, 1, 0 };
	vertices[30].normal = { 0, 1, 0 };
	vertices[31].normal = { 0, 1, 0 };
	vertices[32].normal = { 0, 1, 0 };
	vertices[33].normal = { 0, 0, -1 };
	vertices[34].normal = { 0, 0, -1 };
	vertices[35].normal = { 0, 0, -1 };

	indices = 
	{ 
		0, 1, 3, 
		3, 1, 2,

		1, 5, 2, 
		2, 5, 6,

		5, 4, 6, 
		6, 4, 7,

		4, 0, 7, 
		7, 0, 3,

		3, 2, 7, 
		7, 2, 6,

		4, 5, 0, 
		0, 5, 1
	};
}
