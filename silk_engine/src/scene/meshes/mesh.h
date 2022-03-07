#pragma once

#include "gfx/buffers/vertex_array.h"

class Mesh : NonCopyable
{
	friend class Resources;
	friend class RawModel;
	friend class Scene;

public:
	enum class Type
	{
		NONE,
		_2D,
		_3D
	};

public:
	Mesh(const std::vector<uint32_t>& indices, Type type) : indices(indices), type(type) {}
	virtual ~Mesh() {}
	
	virtual void createVertexArray() = 0;
	virtual void calculateAABB() = 0;
	virtual size_t vertexCount() const = 0;

	bool hasAABB() const { return has_aabb; }

	bool operator==(const Mesh& other) const 
	{ 
		return this == &other;
	}

public:
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;
	Type type = Type::NONE;

protected:
	bool has_aabb = false;
};
