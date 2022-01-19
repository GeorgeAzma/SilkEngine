#pragma once

#include "gfx/vertex_array.h"
#include "scene/vertex.h"

class Mesh : NonCopyable
{
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
	std::string name;
	friend class Resources;
};
