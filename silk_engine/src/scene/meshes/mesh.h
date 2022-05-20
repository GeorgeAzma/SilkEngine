#pragma once

#include "gfx/buffers/vertex_array.h"

class Mesh : NonCopyable
{
public:
	Mesh(const std::vector<uint32_t>& indices = {}) : indices(indices) {}
	virtual ~Mesh() {}
	
	virtual void createVertexArray() = 0;
	virtual void calculateAABB() {}
	virtual size_t vertexCount() const = 0;

	bool hasAABB() const { return has_aabb; }

	bool operator==(const Mesh& other) const 
	{ 
		return this == &other;
	}

public:
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;

protected:
	bool has_aabb = false;
};
