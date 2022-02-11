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
		SK_ASSERT(name != "" && other.name != "", "Mesh compare operator failed, mesh had invalid name"); 
		return name == other.name && *material == *other.material;
	}

public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;
	shared<Material> material = nullptr;

protected:
	void calculateTangents();

private:
	std::string name = "";
	AABB aabb = {};
	bool has_aabb = false;

	friend class Resources;
	friend class RawModel;
	friend class Scene;
};
