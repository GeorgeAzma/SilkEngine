#pragma once

#include "mesh2D.h"

class BezierMesh : public Mesh2D
{
public:
	BezierMesh(float x1, float y1, float px, float py, float x2, float y2, uint32_t resolution = 64, float width = 1.0f, uint32_t edge_resolution = 8, bool tile_UVs = false);
	BezierMesh(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, uint32_t resolution = 64, float width = 1.0f, uint32_t edge_resolution = 8, bool tile_UVs = false);
};