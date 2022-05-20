#pragma once

#include "mesh.h"
#include "scene/AABB.h"

struct Vertex3D
{
	glm::vec3 position = glm::vec3(0);
	glm::vec2 texture_coordinate = glm::vec2(0);
	glm::vec3 normal = glm::vec3(0);
	glm::vec4 color = glm::vec4(1);
};

class Mesh3D : public Mesh
{
	friend class Mesh2D;

public:
	Mesh3D() : Mesh() {}
	Mesh3D(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
	
	void createVertexArray() override;
	void calculateAABB() override;
	size_t vertexCount() const override { return vertices.size(); }

public:
	std::vector<Vertex3D> vertices;

private:
	AABB3D aabb = {};
};
