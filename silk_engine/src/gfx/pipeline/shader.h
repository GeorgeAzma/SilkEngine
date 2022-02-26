#pragma once

#include "gfx/enums.h"
#include "gfx/descriptors/descriptor_set.h"
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

class Shader : NonCopyable
{
	struct Resource
	{
		size_t id;
		uint32_t count = 0;
		uint32_t set = 0;
		uint32_t binding = 0;
		vk::ShaderStageFlags stage_flags;
		vk::DescriptorType type;
	};

	class Includer : public shaderc::CompileOptions::IncluderInterface
	{
		shaderc_include_result* GetInclude(
			const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth) override
		{
			const std::string name = std::string("data/shaders/") + requested_source;
			const std::string contents = File::read(name);

			auto container = new std::array<std::string, 2>;
			(*container)[0] = name;
			(*container)[1] = contents;

			auto data = new shaderc_include_result;

			data->user_data = container;

			data->source_name = (*container)[0].data();
			data->source_name_length = (*container)[0].size();

			data->content = (*container)[1].data();
			data->content_length = (*container)[1].size();

			return data;
		};

		void ReleaseInclude(shaderc_include_result* data) override
		{
			delete static_cast<std::array<std::string, 2>*>(data->user_data);
			delete data;
		};
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

    void compile(const std::vector<Define>& defines = {});

	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return descriptor_sets; }
	const std::vector<vk::PushConstantRange>& getPushConstants() const { return push_constants; }
	const std::vector<vk::PipelineShaderStageCreateInfo>& getPipelineShaderStageInfos() const { return pipeline_shader_stage_infos; }
	const glm::uvec3& getLocalSize() const 
	{ 
		SK_ASSERT(local_size != glm::uvec3(0), "Shader had invalid local size, make sure you are using a compute shader with right local_size."); 
		return local_size; 
	}

public:
	static shaderc_shader_kind shadercType(Type shader_type);
	static shaderc_env_version shadercApiVersion(APIVersion api_version);
    static vk::ShaderStageFlagBits getVulkanType(Type shader_type);
    static Type getStringType(const std::string& shader_string);
    static std::string getTypeFileExtension(Type shader_type);

private:
	std::unordered_map<uint32_t, std::string> parse(const  std::filesystem::path& file);
	vk::ShaderModule createShaderModule(const std::vector<uint32_t>& source) const;
    
	//Reflection
	void loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage_flag, vk::DescriptorType type);
	void loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage_flag);

private:
	std::filesystem::path file;
    std::vector<vk::PipelineShaderStageCreateInfo> pipeline_shader_stage_infos;
	
	//Reflection data
	glm::uvec3 local_size = glm::vec3(0);
	std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
    std::vector<vk::PushConstantRange> push_constants;
	std::vector<Resource> resources;
};