#pragma once

#include "mesh.h"

class RoundedRectangleMesh : public RawMesh2D
{
public:
	RoundedRectangleMesh(uint resolution, float roundness_x, float roundness_y);
	RoundedRectangleMesh(uint resolution = 16, float roundness = 1.0f);
};