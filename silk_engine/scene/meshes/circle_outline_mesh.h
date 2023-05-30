#pragma once

#include "raw_mesh.h"

struct CircleOutlineMesh : public RawMesh2D
{
	CircleOutlineMesh(uint resolution = 64, float thickness = 0.1f);
};

