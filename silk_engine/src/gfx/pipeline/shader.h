#pragma once

#include "gfx/enums.h"
#include "gfx/descriptors/descriptor_set.h"
#include <shaderc/shaderc.hpp>

namespace spirv_cross
{
	class ShaderResources;
	class Compiler;
	class Resource;
}

class Shader : NonCopyable
{
	struct Resource
	{
		size_t id;
		uint32_t count;
		uint32_t set;
		uint32_t binding;
		vk::ShaderStageFlags stage;
		vk::DescriptorType type;
		std::string name;
	};

	struct ResourceLocation
	{
		uint32_t set;
		uint32_t write_index;
	};

	class Includer : public shaderc::CompileOptions::IncluderInterface
	{
		shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override;
		void ReleaseInclude(shaderc_include_result* data) override;
	};

	struct PerStageData
	{
		vk::ShaderStageFlagBits stage;
		std::vector<uint32_t> binary;
		vk::ShaderModule module;
	};

	struct Constant
	{
		uint32_t id = 0;
		vk::ShaderStageFlags stage = vk::ShaderStageFlags(0);
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
    Shader(const std::filesystem::path& file, const std::vector<Define>& defines = {});
	~Shader();

	void compile(const std::vector<Define>& defines = {}, bool force = true);

	void set(std::string_view resource_name, const std::vector<vk::DescriptorBufferInfo>& buffer_infos);
	void set(std::string_view resource_name, const std::vector<vk::DescriptorImageInfo>& image_infos);
	const ResourceLocation& get(std::string_view resource_name) const { return resource_locations.at(resource_name); }
	const ResourceLocation* getIfExists(std::string_view resource_name) const;
	void bindDescriptors();

	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return descriptor_sets; }
	const std::unordered_map<std::string_view, vk::PushConstantRange>& getPushConstants() const { return push_constants; }
	const std::unordered_map<std::string_view, Constant>& getConstants() const { return constants; }
	const std::vector<PerStageData>& getStages() const { return stages; }
	const glm::uvec3& getLocalSize() const 
	{ 
		SK_ASSERT(local_size != glm::uvec3(0), "Shader had invalid local size, make sure you are using a compute shader with right local_size."); 
		return local_size; 
	}

public:
	static shaderc_shader_kind shadercType(Type shader_type);
	static shaderc_env_version shadercApiVersion(APIVersion api_version);
    static vk::ShaderStageFlagBits getVulkanType(Type shader_type);
    static Type getStringType(std::string_view shader_string);
    static std::string getTypeFileExtension(Type shader_type);

private:
	std::unordered_map<uint32_t, std::string> parse(const  std::filesystem::path& file);
    
	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage, vk::DescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage);

private:
	std::filesystem::path file;
	
	//Reflection data
	glm::uvec3 local_size = glm::vec3(0);
	std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
	std::vector<Resource> resources;
    std::unordered_map<std::string_view, vk::PushConstantRange> push_constants;
	std::unordered_map<std::string_view, ResourceLocation> resource_locations;
	std::unordered_map<std::string_view, Constant> constants;
	std::vector<PerStageData> stages;
};