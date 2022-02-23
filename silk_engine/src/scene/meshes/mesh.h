#pragma once

#include "gfx/buffers/vertex_array.h"
#include "scene/vertex.h"
#include "scene/AABB.h"
#include "scene/material.h"

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

protected:
	void calculateTangents();

private:
	AABB aabb = {};
	bool has_aabb = false;

	friend class Resources;
	friend class RawModel;
	friend class Scene;
};
