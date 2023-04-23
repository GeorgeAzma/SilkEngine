#pragma once

#include "gfx/render_context.h"
#include "gfx/descriptors/descriptor_set.h"

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

class Shader : NonCopyable
{
public:
	struct Resource
	{
		size_t id;
		uint32_t count;
		uint32_t set;
		uint32_t binding;
		VkShaderStageFlags stage;
		VkDescriptorType type;
		std::string name;
	};

	struct ResourceLocation
	{
		uint32_t set = std::numeric_limits<uint32_t>::max();
		uint32_t binding = std::numeric_limits<uint32_t>::max();
		operator bool() const { return set != std::numeric_limits<uint32_t>::max(); }
	};

	struct Stage : NonCopyable
	{
	public:
		enum class Type : VkShaderStageFlags
		{
			VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
			FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
			GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
			COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
			TESSELATION_CONTROL = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			TESSELATION_EVALUATION = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
		};

	public:
		Stage(const path& file);
		~Stage();

		bool compile();

		void loadCache();
		void saveCache() const;

		path getCachePath() const;

	private:
		void createModule();

	public:
		path file = "";
		Type type = Type(0);
		VkShaderModule module = nullptr;
		std::vector<uint32_t> binary = {};
	};

	struct Constant
	{
		uint32_t id = 0;
		VkShaderStageFlags stage = VkShaderStageFlags(0);
	};

	struct ReflectionData
	{
		uvec3 local_size = uvec3(0);
		std::unordered_map<uint32_t, shared<DescriptorSetLayout>> descriptor_set_layouts;
		std::vector<Resource> resources;
		std::vector<VkPushConstantRange> push_constants;
		std::unordered_map<std::string_view, VkPushConstantRange> push_constant_map;
		std::unordered_map<std::string_view, ResourceLocation> resource_locations;
		std::unordered_map<std::string_view, Constant> constants;
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions;
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
	};

public:
    Shader(std::string_view name);

	void compile();
	void reflect();

	ResourceLocation getLocation(std::string_view resource_name) const;
	void pushConstants(std::string_view name, const void* data) const;

	const std::vector<unique<Stage>>& getStages() const { return stages; }
	const ReflectionData& getReflectionData() const { return reflection_data; }

private:
	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage, VkDescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage);

private:
	std::vector<unique<Stage>> stages;
	ReflectionData reflection_data{};

public:
	static shared<Shader> get(std::string_view name) { if (auto it = shaders.find(name); it != shaders.end()) return it->second; else return nullptr; }
	static shared<Shader> add(std::string_view name, const shared<Shader> shader) { return shaders.insert_or_assign(name, shader).first->second; }
	static void destroy() { shaders.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Shader>> shaders{};
};