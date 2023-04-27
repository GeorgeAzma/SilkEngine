#pragma once

#include "gfx/buffers/vertex_array.h"

struct RawMesh;

class Mesh : NonCopyable
{
public:
	Mesh(const RawMesh& raw_mesh);

	uint32_t getVertexCount() const { return vertex_array->getVertexCount(); }
	uint32_t getIndexCount() const { return vertex_array->getIndexCount(); }
	const shared<VertexArray>& getVertexArray() const { return vertex_array; }

	operator const shared<VertexArray>&() const { return vertex_array; }
	bool operator==(const Mesh& other) const { return vertex_array == other.vertex_array; }

	void draw(uint32_t first_vertex = 0);
	void drawIndexed(uint32_t first_index = 0, uint32_t vertex_offset = 0);

private:
	shared<VertexArray> vertex_array = nullptr;

public:
	static shared<Mesh> get(std::string_view name) { if (auto it = meshes.find(name); it != meshes.end()) return it->second; else return nullptr; }
	static shared<Mesh> add(std::string_view name, const shared<Mesh> mesh) { return meshes.insert_or_assign(name, mesh).first->second; }
	static void destroy() { meshes.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Mesh>> meshes{};
};