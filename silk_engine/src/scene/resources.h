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

	template<typename T>
	static shared<T> get(std::string_view name);
	static shared<DescriptorSetLayout> getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	template<typename T>
	static void add(std::string_view name, const shared<T>& t);
	static void addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout);

public:
	static ThreadPool pool;
	static inline shared<Image2D> white_image = nullptr;

private:
	template<typename T>
	static inline std::unordered_map<std::string_view, shared<T>> resources;
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