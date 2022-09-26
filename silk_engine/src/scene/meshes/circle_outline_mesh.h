#pragma once

#include "mesh.h"

class CircleOutlineMesh : public RawMesh2D
{
public:
	CircleOutlineMesh(uint resolution = 64, float thickness = 0.2f);
};

