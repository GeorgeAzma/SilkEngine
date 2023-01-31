#pragma once

#include "mesh.h"

class SphereMesh : public RawMesh3D
{
public:
	SphereMesh(uint32_t resolution = 16);

private:
	static vec3 pointOnCubeToPointOnSphere(const vec3& p);
};