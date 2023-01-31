#pragma once

#include "mesh.h"

class TriangleMesh : public RawMesh2D
{
public:
	TriangleMesh();
	TriangleMesh(float x1, float y1, float x2, float y2, float x3, float y3);
};