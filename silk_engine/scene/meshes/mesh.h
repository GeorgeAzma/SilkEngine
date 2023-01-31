#pragma once

#include "raw_mesh.h"
#include "gfx/buffers/vertex_array.h"

class Mesh : NonCopyable
{
public:
	Mesh(const RawMesh& raw_mesh);
	Mesh& operator=(const RawMesh& raw_mesh);

	size_t getVertexCount() const { return vertex_count; }
	size_t getIndexCount() const { return index_count; }
	const shared<VertexArray>& getVertexArray() const { return vertex_array; }

	operator const shared<VertexArray>&() const { return vertex_array; }
	bool operator==(const Mesh& other) const { return vertex_array == other.vertex_array; }

private:
	size_t vertex_count = 0;
	size_t index_count = 0;
	size_t vertex_type_size = 0;
	size_t index_type_size = 0;
	shared<VertexArray> vertex_array = nullptr;
};