#pragma once

#include "mesh.h"

struct Vertex2D
{
	glm::vec2 position = glm::vec3(0);
	glm::vec2 texture_coordinates = glm::vec2(0);
};

class Mesh2D : NonCopyable
{
public:
	Mesh2D() = default;
	Mesh2D(const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);
	virtual ~Mesh2D() {}
	
	void createVertexArray();
	void calculateAABB();
	operator shared<Mesh>() const;

	bool hasAABB() const { return has_aabb; }

	bool operator==(const Mesh2D& other) const 
	{ 
		return this == &other;
	}

public:
	std::vector<Vertex2D> vertices;
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;

private:
	AABB2D aabb = {};
	bool has_aabb = false;

	friend class Resources;
	friend class RawModel;
	friend class Scene;
};
