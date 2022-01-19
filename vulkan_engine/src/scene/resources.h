#pragma once

#include "meshes/mesh.h"
#include "render_object.h"

class Resources
{
public:
	static void init();
	static void cleanup();

	static std::shared_ptr<Mesh> getMesh(const std::string& name);
	static std::shared_ptr<Material> getMaterial(const std::string& name);
	static void addMesh(const std::string& name, std::shared_ptr<Mesh> mesh);
	static void addMaterial(const std::string& name, std::shared_ptr<Material> material);

private:
	static inline std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
	static inline std::unordered_map<std::string, std::shared_ptr<Material>> materials;
};