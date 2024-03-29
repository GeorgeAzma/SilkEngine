#pragma once

class DescriptorSetLayout;

namespace spirv_cross
{
	class ShaderResources;
	class Compiler;
	class Resource;
}

namespace shaderc
{
	class Compiler;
	class CompileOptions;
}

class Shader : NoCopy
{
public:
	using Defines = std::vector<std::pair<std::string, std::string>>;

	struct Stage : NoCopy
	{
	public:
		Stage(const fs::path& file);
		~Stage();

		bool compile(const Defines& defines = {});

		void loadCache();
		void saveCache() const;

		fs::path getCachePath() const;

		bool operator<(const Stage& other) { return type < other.type; }

	private:
		void createModule();

	public:
		fs::path file = "";
		ShaderStage type = ShaderStage(0);
		VkShaderModule module = nullptr;
		std::vector<uint32_t> binary = {};
	};

	struct Constant
	{
		uint32_t id = 0;
		VkShaderStageFlags stage = VkShaderStageFlags(0);
	};

	struct Resource
	{
		size_t id;
		uint32_t count;
		uint32_t set;
		uint32_t binding;
		ShaderStage stage;
		VkDescriptorType type;
		std::string name;
	};

	struct ResourceLocation
	{
		uint32_t set = std::numeric_limits<uint32_t>::max();
		uint32_t binding = std::numeric_limits<uint32_t>::max();
		operator bool() const { return set != std::numeric_limits<uint32_t>::max(); }
	};

	struct ReflectionData
	{
		uvec3 local_size = uvec3(0);
		std::map<uint32_t, shared<DescriptorSetLayout>> descriptor_set_layouts;
		std::vector<Resource> resources;
		std::vector<VkPushConstantRange> push_constants;
		std::unordered_map<std::string_view, ResourceLocation> resource_locations;
		std::unordered_map<std::string, Constant> constants;
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions;
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
		std::unordered_map<uint32_t, std::string_view> render_targets;
	};

public:
    Shader(std::string_view name, const Defines& defines = {});
	Shader(const std::vector<std::string_view>& names, const Defines& defines = {});

	void compile(const Defines& defines = {});
	void reflect();

	ResourceLocation getLocation(std::string_view resource_name) const;

	const std::vector<unique<Stage>>& getStages() const { return stages; }
	const ReflectionData& getReflectionData() const { return reflection_data; }

private:
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, ShaderStage stage, VkDescriptorType type);
	
private:
	std::vector<unique<Stage>> stages;
	ReflectionData reflection_data{};

public:
	static shared<Shader> get(std::string_view name) { if (auto it = shaders.find(name); it != shaders.end()) return it->second; return nullptr; }
	static shared<Shader> add(std::string_view name, const shared<Shader>& shader) { return shaders.insert_or_assign(name, shader).first->second; }
	static void destroy() { shaders.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Shader>> shaders{};
};