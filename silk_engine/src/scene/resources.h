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
	static shared<Image> getImage(const std::string& name);
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	static void addMesh(const std::string& name, shared<Mesh> mesh);
	static void addMaterial(const std::string& name, shared<Material> material);
	static void addComputeMaterial(const std::string& name, shared<ComputeMaterial> compute_material);
	static void addImage(const std::string& name, shared<Image> image);
	static void addDescriptorSetLayout(shared<DescriptorSetLayout> descriptor_layout);

private:
	static inline std::unordered_map<std::string, shared<Mesh>> meshes;
	static inline std::unordered_map<std::string, shared<Material>> materials;
	static inline std::unordered_map<std::string, shared<ComputeMaterial>> compute_materials;
	static inline std::unordered_map<std::string, shared<Image>> images;

	struct DescriptorSetLayoutInfo
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings; 

		bool operator==(const DescriptorSetLayoutInfo& other) const;
		size_t hash() const;
	};
	struct DescriptorSetLayoutHash 
	{
		size_t operator()(const DescriptorSetLayoutInfo& k) const { return k.hash(); }
	};
	static inline std::unordered_map<DescriptorSetLayoutInfo, shared<DescriptorSetLayout>, DescriptorSetLayoutHash> descriptor_set_layouts;
};