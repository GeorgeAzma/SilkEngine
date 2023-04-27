#pragma once

#include "raw_mesh.h"

struct SphereMesh : public RawMesh3D
{
	SphereMesh(uint32_t resolution = 16);

private:
	static vec3 pointOnCubeToPointOnSphere(const vec3& p);
};