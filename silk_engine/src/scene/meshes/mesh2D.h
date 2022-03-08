#pragma once

#include "mesh3D.h"
#include "scene/AABB.h"

struct Vertex2D
{
	glm::vec2 position = glm::vec2(0);
	glm::vec2 texture_coordinates = glm::vec2(0);
};

class Mesh2D : public Mesh
{
public:
	Mesh2D() : Mesh({}, Type::_2D) {}
	Mesh2D(const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);
	
	void createVertexArray() override;
	void calculateAABB() override;
	size_t vertexCount() const override { return vertices.size(); }
	operator shared<Mesh3D>() const;

protected:
	std::vector<Vertex2D> vertices;
	AABB2D aabb = {};
};
