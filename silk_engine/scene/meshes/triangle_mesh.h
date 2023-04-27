#pragma once

#include "raw_mesh.h"

struct TriangleMesh : public RawMesh2D
{
	TriangleMesh();
	TriangleMesh(float x1, float y1, float x2, float y2, float x3, float y3);
};