#pragma once

#include "meshes/mesh.h"
#include "model.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/ui/font.h"
#include "gfx/allocators/command_pool.h"

class Resources
{
	template<typename T>
	struct Resource
	{
		Resource(const std::function<shared<T>()>& fetch_function = nullptr) 
			: fetch_function(fetch_function) {}
		std::function<shared<T>()> fetch_function = nullptr;
		shared<T> resource = nullptr;
		shared<T> fetch()
		{
			if(!resource.get())
				return (resource = fetch_function());
			return resource;
		}
	};

public:
	static void init();
	static void cleanup();

	static shared<Mesh> getMesh(const std::string& name);
	static shared<Model> getModel(const std::string& name);
	static shared<Shader> getShader(const std::string& name);
	static shared<GraphicsPipeline> getGraphicsPipeline(const std::string& name);
	static shared<ComputePipeline> getComputePipeline(const std::string& name);
	static shared<Image2D> getImage(const std::string& name);
	static shared<Font> getFont(const std::string& path);
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

	static void addMesh(const std::string& name, const std::function<shared<Mesh>()>& mesh);
	static void addModel(const std::string& name, const std::function<shared<Model>()>& model);
	static void addShader(const std::string& name, const std::function<shared<Shader>()>& shader);
	static void addGraphicsPipeline(const std::string& name, const std::function<shared<GraphicsPipeline>()>& graphics_pipeline);
	static void addComputePipeline(const std::string& name, const std::function<shared<ComputePipeline>()>& compute_pipeline);
	static void addImage(const std::string& name, const std::function<shared<Image2D>()>& image);
	static void addFont(const std::string& name, const std::function<shared<Font>()>& font);
	static void addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout);

public:
	static inline ThreadPool pool;
	static inline shared<Image2D> white_image = nullptr;

private:
	template<typename T>
	static shared<T> fetch(std::unordered_map<std::string, Resource<T>>& resources, const std::string& name) 
	{ 
		auto it = resources.find(name);
		return it == resources.end() ? nullptr : it->second.fetch();
	}

	template<typename T>
	static void add(std::unordered_map<std::string, Resource<T>>& resources, const std::string& name, const std::function<shared<T>()>& resource_fetch_func)
	{
		resources[name] = Resource<T>(resource_fetch_func);
	}

private:
	static inline std::unordered_map<std::string, Resource<Mesh>> meshes;
	static inline std::unordered_map<std::string, Resource<Model>> models;
	static inline std::unordered_map<std::string, Resource<Shader>> shaders;
	static inline std::unordered_map<std::string, Resource<GraphicsPipeline>> graphics_pipelines;
	static inline std::unordered_map<std::string, Resource<ComputePipeline>> compute_pipelines;
	static inline std::unordered_map<std::string, Resource<Image2D>> images;
	static inline std::unordered_map<std::string, Resource<Font>> fonts;
	struct DescriptorSetLayoutInfo
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorSetLayoutInfo& other) const;
		size_t hash() const;
	};
	struct DescriptorSetLayoutHash 
	{
		size_t operator()(const DescriptorSetLayoutInfo& k) const { return k.hash(); }
	};
	static inline std::unordered_map<DescriptorSetLayoutInfo, shared<DescriptorSetLayout>, DescriptorSetLayoutHash> descriptor_set_layouts;
};