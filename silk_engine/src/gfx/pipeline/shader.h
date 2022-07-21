#pragma once

#include "gfx/graphics.h"
#include "gfx/descriptors/descriptor_set.h"
#include <shaderc/shaderc.hpp>

namespace spirv_cross
{
	class ShaderResources;
	class Compiler;
	class Resource;
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

	class Includer : public shaderc::CompileOptions::IncluderInterface
	{
		shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override;
		void ReleaseInclude(shaderc_include_result* data) override;
	};

	struct PerStageData
	{
		VkShaderStageFlagBits stage;
		std::vector<uint32_t> binary;
		VkShaderModule module;
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
		std::array<std::optional<bool>, (size_t)EnableTag::LAST> enabled{};
	};

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

	struct Define
	{
		std::string name = "";
		std::string value = "";
	};

public:
    Shader(std::string_view file, const std::vector<Define>& defines = {});
	~Shader();

	void compile(const std::vector<Define>& defines = {}, bool force = true);
	void reflect();

	void set(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void set(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos);
	void setIfExists(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos);
	const ResourceLocation& get(std::string_view resource_name) const { return resource_locations.at(resource_name); }
	const ResourceLocation* getIfExists(std::string_view resource_name) const;
	void bindDescriptorSets();

	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return descriptor_sets; }
	const std::unordered_map<std::string_view, VkPushConstantRange>& getPushConstants() const { return push_constants; }
	const std::unordered_map<std::string, Constant>& getConstants() const { return constants; }
	const Parameters& getParameters() const { return parameters; }
	const std::vector<PerStageData>& getStages() const { return stages; }
	const glm::uvec3& getLocalSize() const 
	{ 
		SK_ASSERT(local_size != glm::uvec3(0), "Shader had invalid local size, make sure you are using a compute shader with right local_size."); 
		return local_size; 
	}

public:
	static shaderc_shader_kind shadercType(Type shader_type);
	static shaderc_env_version shadercApiVersion(APIVersion api_version);
    static VkShaderStageFlagBits getVulkanType(Type shader_type);
    static Type getStringType(std::string_view shader_string);
    static std::string getTypeFileExtension(Type shader_type);

private:
	std::unordered_map<uint32_t, std::string> parse(const  std::filesystem::path& file);
    
	static shaderc::CompileOptions getCompileOptions();
	template<typename T>
	static void updateParameter(const std::string& source, T& parameter_value, const std::function<T(std::string_view)>& update_function, const char* parameter_name);
	template<typename T>
	static void updateParameter(const std::string& source, T& parameter_value, const char* parameter_name, const std::vector<std::pair<const char*, T>>& value_pairs);

	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage, VkDescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage);

private:
	std::filesystem::path file;
	
	//Reflection data
	glm::uvec3 local_size = glm::vec3(0);
	std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
	std::vector<Resource> resources;
    std::unordered_map<std::string_view, VkPushConstantRange> push_constants;
	std::unordered_map<std::string_view, ResourceLocation> resource_locations;
	std::unordered_map<std::string, Constant> constants;
	std::vector<PerStageData> stages;
	Parameters parameters{};
};