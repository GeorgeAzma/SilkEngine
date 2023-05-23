#pragma once

#include "silk_engine/gfx/buffers/vertex_array.h"

struct RawMesh;

class Mesh : public VertexArray
{
public:
	using VertexArray::VertexArray;

public:
	static shared<Mesh> get(std::string_view name) 
	{ 
		if (auto it = meshes.find(name); it != meshes.end()) 
			return it->second; 
		return nullptr; 
	}
	static shared<Mesh> add(std::string_view name, const shared<Mesh>& mesh) { return meshes.insert_or_assign(name, mesh).first->second; }
	static void destroy() { meshes.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Mesh>> meshes{};
};