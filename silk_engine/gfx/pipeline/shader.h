#pragma once

#include "gfx/render_context.h"
#include "gfx/descriptors/descriptor_set.h"
#include "gfx/buffers/buffer_layout.h"

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
		uint32_t set;
		uint32_t binding;
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
		uvec3 local_size = vec3(0);
		std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		std::vector<Resource> resources;
		std::vector<VkPushConstantRange> push_constants;
		std::unordered_map<std::string_view, VkPushConstantRange> push_constant_map;
		std::unordered_map<std::string_view, ResourceLocation> resource_locations;
		std::unordered_map<std::string_view, Constant> constants;
		BufferLayout buffer_layout{};
	};

public:
    Shader(std::string_view name);

	void compile();
	void reflect();

	void set(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void set(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	const ResourceLocation& get(std::string_view resource_name) const { return reflection_data.resource_locations.at(resource_name); }
	const ResourceLocation* getIfExists(std::string_view resource_name) const;
	void bindDescriptorSets();
	void pushConstants(std::string_view name, const void* data) const;

	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return reflection_data.descriptor_sets; }
	const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const { return reflection_data.descriptor_set_layouts; }
	const std::vector<VkPushConstantRange>& getPushConstants() const { return reflection_data.push_constants; }
	const std::unordered_map<std::string_view, Constant>& getConstants() const { return reflection_data.constants; }
	const std::vector<unique<Stage>>& getStages() const { return stages; }
	const BufferLayout& getBufferLayout() const { return reflection_data.buffer_layout; }
	const uvec3& getLocalSize() const { return reflection_data.local_size; }

private:
	static std::string_view getPreprocessorValue(std::string_view source, std::string_view preprocessor_name, size_t offset = 0);
	template<typename T>
	static void updateParameter(std::string_view source, T& parameter_value, const std::function<T(std::string_view)>& update_function, std::string_view parameter_name);
	template<typename T>
	static void updateParameter(std::string_view source, T& parameter_value, std::string_view parameter_name, const std::vector<std::pair<std::string_view, T>>& value_pairs);

	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage, VkDescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage);

private:
	std::vector<unique<Stage>> stages;
	ReflectionData reflection_data{};
	static shaderc::Compiler compiler;
};