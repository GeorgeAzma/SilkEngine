#pragma once

#include "mesh2D.h"

class TriangleMesh : public Mesh2D
{
public:
	TriangleMesh();
	TriangleMesh(float x1, float y1, float x2, float y2, float x3, float y3);
};