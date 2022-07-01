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
public:
	static void init();
	static void destroy();

	static shared<Mesh> getMesh(std::string_view name);
	static shared<Model> getModel(std::string_view name);
	static shared<Shader> getShader(std::string_view name);
	static shared<GraphicsPipeline> getGraphicsPipeline(std::string_view name);
	static shared<ComputePipeline> getComputePipeline(std::string_view name);
	static shared<Image2D> getImage(std::string_view name);
	static shared<Font> getFont(std::string_view path);
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	static void addMesh(std::string_view name, const shared<Mesh>& mesh);
	static void addModel(std::string_view name, const shared<Model>& model);
	static void addShader(std::string_view name, const shared<Shader>& shader);
	static void addGraphicsPipeline(std::string_view name, const shared<GraphicsPipeline>& graphics_pipeline);
	static void addComputePipeline(std::string_view name, const shared<ComputePipeline>& compute_pipeline);
	static void addImage(std::string_view name, const shared<Image2D>& image);
	static void addFont(std::string_view name, const shared<Font>& font);
	static void addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout);

public:
	static ThreadPool pool;
	static inline shared<Image2D> white_image = nullptr;

private:
	static inline std::unordered_map<std::string_view, std::shared_ptr<Mesh>> meshes;
	static inline std::unordered_map<std::string_view, std::shared_ptr<Model>> models;
	static inline std::unordered_map<std::string_view, std::shared_ptr<Shader>> shaders;
	static inline std::unordered_map<std::string_view, std::shared_ptr<GraphicsPipeline>> graphics_pipelines;
	static inline std::unordered_map<std::string_view, std::shared_ptr<ComputePipeline>> compute_pipelines;
	static inline std::unordered_map<std::string_view, std::shared_ptr<Image2D>> images;
	static inline std::unordered_map<std::string_view, std::shared_ptr<Font>> fonts;
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