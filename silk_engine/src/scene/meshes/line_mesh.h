#pragma once

#include "mesh.h"

class LineMesh : public RawMesh2D
{
public:
	LineMesh(const std::vector<vec2>& points, float width = 1.0f, uint32_t edge_resolution = 8, bool tile_UVs = false);
};