#pragma once

#include "mesh2D.h"

class LineMesh : public Mesh2D
{
public:
	LineMesh(const std::vector<glm::vec2>& points, float width = 1.0f, uint32_t edge_resolution = 8, bool tile_UVs = false);
};