#pragma once

#include "raw_mesh.h"

struct RoundedRectangleMesh : public RawMesh2D
{
	RoundedRectangleMesh(uint resolution, float roundness_x, float roundness_y);
	RoundedRectangleMesh(uint resolution = 16, float roundness = 1.0f);
};