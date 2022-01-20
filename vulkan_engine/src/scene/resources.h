#pragma once

#include "meshes/mesh.h"
#include "render_object.h"

class Resources
{
public:
	static void init();
	static void cleanup();

	static shared<Mesh> getMesh(const std::string& name);
	static shared<Material> getMaterial(const std::string& name);
	static shared<ComputeMaterial> getComputeMaterial(const std::string& name);
	static void addMesh(const std::string& name, shared<Mesh> mesh);
	static void addMaterial(const std::string& name, shared<Material> material);
	static void addComputeMaterial(const std::string& name, shared<ComputeMaterial> compute_material);

private:
	static inline std::unordered_map<std::string, shared<Mesh>> meshes;
	static inline std::unordered_map<std::string, shared<Material>> materials;
	static inline std::unordered_map<std::string, shared<ComputeMaterial>> compute_materials;
};