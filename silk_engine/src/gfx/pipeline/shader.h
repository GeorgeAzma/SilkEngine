#pragma once

#include "gfx/graphics.h"
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

enum class EnableTag // Do not adjust order
{
	DEPTH_TEST,
	DEPTH_WRITE,
	STENCIL_TEST,
	BLEND,
	SAMPLE_SHADING,
	PRIMITIVE_RESTART,
	RASTERIZER_DISCARD,
	DEPTH_CLAMP,
	DEPTH_BIAS,
	COLOR_BLEND_LOGIC_OP,

	LAST
};

class Shader : NonCopyable
{
public:
	struct Define
	{
		std::string name = "";
		std::string value = "";
	};

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

	struct Stage
	{
	public:
		enum class Type : uint32_t
		{
			NONE = 0,
			VERTEX = 1,
			FRAGMENT = 2,
			GEOMETRY = 4,
			COMPUTE = 8,
			TESSELATION_CONTROL = 16,
			TESSELATION_EVALUATION = 32
		};

	public:
		bool compile(std::string_view source, const shaderc::Compiler& compiler, const shaderc::CompileOptions& options, bool skip_if_error = false);
		void createModule();
		std::string getCachePath() const { return std::string("res/cache/shaders") + '/' + file + ".glsl" + '.' + getTypeFileExtension(type) + ".spv"; }
		void loadCache();
		void saveCache() const;
		void destroy();

		static VkShaderStageFlagBits toVulkanType(Type shader_type);
		static Type fromVulkanType(VkShaderStageFlagBits vulkan_type);
		static Type getStringType(std::string_view shader_string);
		static std::string getTypeFileExtension(Type shader_type);

	public:
		std::string file = "";
		Type type = Type::NONE;
		VkShaderModule module = nullptr;
		std::vector<uint32_t> binary = {};
	};

	struct Constant
	{
		uint32_t id = 0;
		VkShaderStageFlags stage = VkShaderStageFlags(0);
	};

	struct Parameters
	{
		//blend (src, dst)
		std::optional<VkCullModeFlags> cull_mode{};
		std::optional<float> line_width{};
		std::optional<VkPolygonMode> polygon_mode{};
		std::optional<VkFrontFace> front_face{};
		std::optional<float> depth_bias{};
		std::optional<float> depth_slope{};
		std::optional<float> depth_clamp{};
		std::optional<VkCompareOp> depth_compare_op{};
		std::optional<VkBlendFactor> src_blend{};
		std::optional<VkBlendFactor> dst_blend{};
		std::optional<VkBlendOp> blend_op{};
		std::optional<VkBlendFactor> src_alpha_blend{};
		std::optional<VkBlendFactor> dst_alpha_blend{};
		std::optional<VkBlendOp> alpha_blend_op{};
		std::optional<uint32_t> color_write_mask{};
		std::optional<uint32_t> subpass{};
		std::array<std::optional<bool>, (size_t)EnableTag::LAST> enabled{};
	};

	struct ReflectionData
	{
		uvec3 local_size = vec3(0);
		std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
		std::vector<Resource> resources;
		std::unordered_map<std::string_view, VkPushConstantRange> push_constants;
		std::unordered_map<std::string_view, ResourceLocation> resource_locations;
		std::unordered_map<std::string, Constant> constants;
		BufferLayout buffer_layout{};
	};

public:
    Shader(std::string_view file, const std::vector<Define>& defines = {});
	~Shader();

	void compile();
	void reflect();

	void set(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void set(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	const ResourceLocation& get(std::string_view resource_name) const { return reflection_data.resource_locations.at(resource_name); }
	const ResourceLocation* getIfExists(std::string_view resource_name) const;
	void bindDescriptorSets();

	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return reflection_data.descriptor_sets; }
	const std::unordered_map<std::string_view, VkPushConstantRange>& getPushConstants() const { return reflection_data.push_constants; }
	const std::unordered_map<std::string, Constant>& getConstants() const { return reflection_data.constants; }
	const Parameters& getParameters() const { return parameters; }
	const std::vector<Stage>& getStages() const { return stages; }
	const BufferLayout& getBufferLayout() const { return reflection_data.buffer_layout; }
	const uvec3& getLocalSize() const 
	{ 
		SK_ASSERT(reflection_data.local_size != uvec3(0), "Shader had invalid local size, make sure you are using a compute shader with right local_size.");
		return reflection_data.local_size;
	}

private:
	std::unordered_map<uint32_t, std::string> parse(std::string_view file, Parameters& parameters);
    
	static std::string_view getPreprocessorValue(std::string_view source, std::string_view preprocessor_name, size_t offset = 0);
	template<typename T>
	static void updateParameter(std::string_view source, T& parameter_value, const std::function<T(std::string_view)>& update_function, std::string_view parameter_name);
	template<typename T>
	static void updateParameter(std::string_view source, T& parameter_value, std::string_view parameter_name, const std::vector<std::pair<std::string_view, T>>& value_pairs);

	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage, VkDescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage);

private:
	std::string file;
	std::vector<Stage> stages;
	std::vector<Define> defines;
	Parameters parameters{};
	double last_compiled = 0.0;
	
	ReflectionData reflection_data{};
};