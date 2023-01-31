#include "cube_mesh.h"

CubeMesh::CubeMesh()
{
	//UNTESTED: Most likely false
	resizeVertices(36);
	resizeIndices(36);
		
	getVertex(0 ).position = { 0, 0, 0 };
	getVertex(1 ).position = { 1, 0, 0 };
	getVertex(2 ).position = { 0, 1, 0 };
	getVertex(3 ).position = { 0, 1, 0 };
	getVertex(4 ).position = { 1, 0, 0 };
	getVertex(5 ).position = { 1, 1, 0 };

	getVertex(6 ).position = { 1, 0, 0 };
	getVertex(7 ).position = { 1, 0, 1 };
	getVertex(8 ).position = { 1, 1, 0 };
	getVertex(9 ).position = { 1, 1, 0 };
	getVertex(10).position = { 1, 0, 1 };
	getVertex(11).position = { 1, 1, 1 };

	getVertex(12).position = { 1, 0, 1 };
	getVertex(13).position = { 0, 0, 1 };
	getVertex(14).position = { 1, 1, 1 };
	getVertex(15).position = { 1, 1, 1 };
	getVertex(16).position = { 0, 0, 1 };
	getVertex(17).position = { 0, 1, 1 };

	getVertex(18).position = { 0, 0, 1 };
	getVertex(19).position = { 0, 0, 0 };
	getVertex(20).position = { 0, 1, 1 };
	getVertex(21).position = { 0, 1, 1 };
	getVertex(22).position = { 0, 0, 0 };
	getVertex(23).position = { 0, 1, 0 };

	getVertex(24).position = { 0, 1, 0 };
	getVertex(25).position = { 1, 1, 0 };
	getVertex(26).position = { 0, 1, 1 };
	getVertex(27).position = { 0, 1, 1 };
	getVertex(28).position = { 1, 1, 0 };
	getVertex(29).position = { 1, 1, 1 };

	getVertex(30).position = { 0, 0, 1 };
	getVertex(31).position = { 1, 0, 1 };
	getVertex(32).position = { 0, 0, 0 };
	getVertex(33).position = { 0, 0, 0 };
	getVertex(34).position = { 1, 0, 1 };
	getVertex(35).position = { 1, 0, 0 };

	getVertex(0 ).texture_coordinate = { 0, 0 };
	getVertex(1 ).texture_coordinate = { 1, 0 };
	getVertex(2 ).texture_coordinate = { 0, 1 };
	getVertex(3 ).texture_coordinate = { 0, 1 };
	getVertex(4 ).texture_coordinate = { 1, 0 };
	getVertex(5 ).texture_coordinate = { 1, 1 };

	getVertex(6 ).texture_coordinate = { 0, 0 };
	getVertex(7 ).texture_coordinate = { 1, 0 };
	getVertex(8 ).texture_coordinate = { 0, 1 };
	getVertex(9 ).texture_coordinate = { 0, 1 };
	getVertex(10).texture_coordinate = { 1, 0 };
	getVertex(11).texture_coordinate = { 1, 1 };

	getVertex(12).texture_coordinate = { 0, 0 };
	getVertex(13).texture_coordinate = { 1, 0 };
	getVertex(14).texture_coordinate = { 0, 1 };
	getVertex(15).texture_coordinate = { 0, 1 };
	getVertex(16).texture_coordinate = { 1, 0 };
	getVertex(17).texture_coordinate = { 1, 1 };

	getVertex(18).texture_coordinate = { 0, 0 };
	getVertex(19).texture_coordinate = { 1, 0 };
	getVertex(20).texture_coordinate = { 0, 1 };
	getVertex(21).texture_coordinate = { 0, 1 };
	getVertex(22).texture_coordinate = { 1, 0 };
	getVertex(23).texture_coordinate = { 1, 1 };

	getVertex(24).texture_coordinate = { 0, 0 };
	getVertex(25).texture_coordinate = { 1, 0 };
	getVertex(26).texture_coordinate = { 0, 1 };
	getVertex(27).texture_coordinate = { 0, 1 };
	getVertex(28).texture_coordinate = { 1, 0 };
	getVertex(29).texture_coordinate = { 1, 1 };

	getVertex(30).texture_coordinate = { 0, 0 };
	getVertex(31).texture_coordinate = { 1, 0 };
	getVertex(32).texture_coordinate = { 0, 1 };
	getVertex(33).texture_coordinate = { 0, 1 };
	getVertex(34).texture_coordinate = { 1, 0 };
	getVertex(35).texture_coordinate = { 1, 1 };

	getVertex(0 ).normal = {  0,  0, -1 };
	getVertex(1 ).normal = {  0,  0, -1 };
	getVertex(2 ).normal = {  0,  0, -1 };
	getVertex(3 ).normal = {  0,  0, -1 };
	getVertex(4 ).normal = {  0,  0, -1 };
	getVertex(5 ).normal = {  0,  0, -1 };
	getVertex(6 ).normal = {  1,  0,  0 };
	getVertex(7 ).normal = {  1,  0,  0 };
	getVertex(8 ).normal = {  1,  0,  0 };
	getVertex(9 ).normal = {  1,  0,  0 };
	getVertex(10).normal = {  1,  0,  0 };
	getVertex(11).normal = {  1,  0,  0 };
	getVertex(12).normal = {  0,  0,  1 };
	getVertex(13).normal = {  0,  0,  1 };
	getVertex(14).normal = {  0,  0,  1 };
	getVertex(15).normal = {  0,  0,  1 };
	getVertex(16).normal = {  0,  0,  1 };
	getVertex(17).normal = {  0,  0,  1 };
	getVertex(18).normal = { -1,  0,  0 };
	getVertex(19).normal = { -1,  0,  0 };
	getVertex(20).normal = { -1,  0,  0 };
	getVertex(21).normal = { -1,  0,  0 };
	getVertex(22).normal = { -1,  0,  0 };
	getVertex(23).normal = { -1,  0,  0 };
	getVertex(24).normal = {  0,  1,  0 };
	getVertex(25).normal = {  0,  1,  0 };
	getVertex(26).normal = {  0,  1,  0 };
	getVertex(27).normal = {  0,  1,  0 };
	getVertex(28).normal = {  0,  1,  0 };
	getVertex(29).normal = {  0,  1,  0 };
	getVertex(30).normal = {  0, -1,  0 };
	getVertex(31).normal = {  0, -1,  0 };
	getVertex(32).normal = {  0, -1,  0 };
	getVertex(33).normal = {  0, -1,  0 };
	getVertex(34).normal = {  0, -1,  0 };
	getVertex(35).normal = {  0, -1,  0 };

	getIndex(0) = 0; getIndex(1) = 1; getIndex(2) = 2;
	getIndex(3) = 3; getIndex(4) = 4; getIndex(5) = 5;

	getIndex(6) = 6; getIndex(7) = 7; getIndex(8) = 8;
	getIndex(9) = 9; getIndex(10) = 10; getIndex(11) = 11;
	
	getIndex(12) = 12; getIndex(13) = 13; getIndex(14) = 14;
	getIndex(15) = 15; getIndex(16) = 16; getIndex(17) = 17;
	
	getIndex(18) = 18; getIndex(19) = 19; getIndex(20) = 20;
	getIndex(21) = 21; getIndex(22) = 22; getIndex(23) = 23;
	
	getIndex(24) = 24; getIndex(25) = 25; getIndex(26) = 26;
	getIndex(27) = 27; getIndex(28) = 28; getIndex(29) = 29;
	
	getIndex(30) = 30; getIndex(31) = 31; getIndex(32) = 32;
	getIndex(33) = 33; getIndex(34) = 34; getIndex(35) = 35;
}
