#pragma once

#include "meshes/mesh.h"

class Resources
{
public:
	static void init();
	static void cleanup();

	static std::shared_ptr<Mesh> getMesh(const char* name);
	static void addMesh(const char* name, std::shared_ptr<Mesh> mesh);

private:
	static inline std::unordered_map<const char*, std::shared_ptr<Mesh>> meshes;
};