#pragma once

#include "gfx/buffers/vertex_array.h"
#include "scene/vertex.h"
#include "scene/AABB.h"
#include "scene/material.h"

class Mesh : NonCopyable
{
	static constexpr size_t MAX_VERTEX_COUNT_TO_CALCULATE_BOUNDS = 0; //We don't do culling for now, so no need for bounds
public:
	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	virtual ~Mesh() {}
	
	void createVertexArray();
	void calculateAABB();

	bool operator==(const Mesh& other) const 
	{ 
		SK_ASSERT(name != "" && other.name != "", "Mesh compare operator failed, mesh had invalid name"); 
		return name == other.name; 
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

	friend class Resources;
	friend class Model;
	friend class Scene;
};
