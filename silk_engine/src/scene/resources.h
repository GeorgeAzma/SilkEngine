#pragma once

#include "meshes/mesh.h"
#include "model.h"
#include "material.h"
#include "gfx/ui/font.h"
#include "gfx/allocators/command_pool.h"

class Resources
{
public:
	static void init();
	static void cleanup();

	static shared<Mesh> getMesh(const std::string& name);
	static shared<Model> getModel(const std::string& name);
	static shared<ShaderEffect> getShaderEffect(const std::string& name);
	static shared<Material> getMaterial(const std::string& name);
	static shared<ComputeShaderEffect> getComputeShaderEffect(const std::string& name);
	static shared<Image2D> getImage(const std::string& name);
	//@return descriptor layout from the cache, if it doesn't exist creates new one
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	static shared<Font> getFont(const std::string& path);

	static void addMesh(const std::string& name, shared<Mesh> mesh);
	static void addModel(const std::string& name, shared<Model> model);
	static void addShaderEffect(const std::string& name, shared<ShaderEffect> shader_effect);
	static void addMaterial(const std::string& name, shared<Material> material);
	static void addComputeShaderEffect(const std::string& name, shared<ComputeShaderEffect> compute_shader_effect);
	static void addImage(const std::string& name, shared<Image2D> image);
	static void addDescriptorSetLayout(shared<DescriptorSetLayout> descriptor_layout);
	static void addFont(const std::string& name, shared<Font> font);

public:
	static inline ThreadPool pool;

private:
	static inline std::unordered_map<std::string, shared<Mesh>> meshes;
	static inline std::unordered_map<std::string, shared<Model>> models;
	static inline std::unordered_map<std::string, shared<ShaderEffect>> shader_effects;
	static inline std::unordered_map<std::string, shared<Material>> materials;
	static inline std::unordered_map<std::string, shared<ComputeShaderEffect>> compute_shader_effects;
	static inline std::unordered_map<std::string, shared<Image2D>> images;
	static inline std::unordered_map<std::string, shared<Font>> fonts;
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