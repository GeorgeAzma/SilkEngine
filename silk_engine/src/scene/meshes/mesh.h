#pragma once

#include "gfx/buffers/vertex_array.h"
#include "scene/vertex.h"
#include "scene/AABB.h"

class Mesh : NonCopyable
{
	static constexpr size_t MAX_VERTEX_COUNT_TO_CALCULATE_BOUNDS = 0; //We don't do culling for now, so no need for bounds
public:
	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	virtual ~Mesh() {}
	
	bool operator==(const Mesh& other) const { return name == other.name; }

protected:
	void init();

public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;

protected:
	void calculateTangents();

private:
	std::string name = "";
	AABB aabb;

	friend class Resources;
	friend class Model;
	friend class Scene;
};
