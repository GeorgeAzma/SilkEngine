#pragma once

class Mesh;
class Model;
class Font;
class GraphicsPipeline;
class ComputePipeline;
class ThreadPool;
class DescriptorSetLayout;
class Image2D;
class Shader;

class Resources
{
	template<typename T>
	struct Resource
	{
		Resource(const std::function<shared<T>()>& fetch_function = nullptr) 
			: fetch_function(fetch_function) {}
		std::function<shared<T>()> fetch_function = nullptr;
		shared<T> resource = nullptr;
		shared<T>& fetch()
		{
			if(resource.get())
				return resource;
			SK_ASSERT(fetch_function != nullptr, "Couldn't fetch resource, because fetch function provided was nullptr");
			return (resource = fetch_function());
		}
	};

public:
	static void init();
	static void cleanup();

	static shared<Mesh> getMesh(std::string_view name);
	static shared<Model> getModel(std::string_view name);
	static shared<Shader> getShader(std::string_view name);
	static shared<GraphicsPipeline> getGraphicsPipeline(std::string_view name);
	static shared<ComputePipeline> getComputePipeline(std::string_view name);
	static shared<Image2D> getImage(std::string_view name);
	static shared<Font> getFont(std::string_view path);
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	static void addMesh(std::string_view name, const std::function<shared<Mesh>()>& mesh);
	static void addModel(std::string_view name, const std::function<shared<Model>()>& model);
	static void addShader(std::string_view name, const std::function<shared<Shader>()>& shader);
	static void addGraphicsPipeline(std::string_view name, const std::function<shared<GraphicsPipeline>()>& graphics_pipeline);
	static void addComputePipeline(std::string_view name, const std::function<shared<ComputePipeline>()>& compute_pipeline);
	static void addImage(std::string_view name, const std::function<shared<Image2D>()>& image);
	static void addFont(std::string_view name, const std::function<shared<Font>()>& font);
	static void addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout);

public:
	static ThreadPool pool;
	static inline shared<Image2D> white_image = nullptr;

private:
	template<typename T>
	static shared<T> fetch(std::unordered_map<std::string_view, Resource<T>>& resources, std::string_view name)
	{ 
		auto it = resources.find(name);
		return it != resources.end() ? it->second.fetch() : nullptr;
	}

	template<typename T>
	static void add(std::unordered_map<std::string_view, Resource<T>>& resources, std::string_view name, const std::function<shared<T>()>& resource_fetch_func)
	{
		resources[name] = Resource<T>(resource_fetch_func);
	}

private:
	static inline std::unordered_map<std::string_view, Resource<Mesh>> meshes;
	static inline std::unordered_map<std::string_view, Resource<Model>> models;
	static inline std::unordered_map<std::string_view, Resource<Shader>> shaders;
	static inline std::unordered_map<std::string_view, Resource<GraphicsPipeline>> graphics_pipelines;
	static inline std::unordered_map<std::string_view, Resource<ComputePipeline>> compute_pipelines;
	static inline std::unordered_map<std::string_view, Resource<Image2D>> images;
	static inline std::unordered_map<std::string_view, Resource<Font>> fonts;
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