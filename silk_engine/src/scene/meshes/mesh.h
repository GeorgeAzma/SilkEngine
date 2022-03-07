#pragma once

#include "gfx/buffers/vertex_array.h"
#include "scene/AABB.h"
#include "scene/material.h"

struct Vertex
{
	glm::vec3 position = glm::vec3(0);
	glm::vec2 texture_coordinates = glm::vec2(0);
	glm::vec3 normal = glm::vec3(0);
};

class Mesh : NonCopyable
{
public:
	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	virtual ~Mesh() {}
	
	void createVertexArray();
	void calculateAABB();

	bool hasAABB() const { return has_aabb; }

	bool operator==(const Mesh& other) const 
	{ 
		return this == &other;
	}

public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;

private:
	AABB aabb = {};
	bool has_aabb = false;

	friend class Resources;
	friend class RawModel;
	friend class Scene;
	friend class Mesh2D;
};
