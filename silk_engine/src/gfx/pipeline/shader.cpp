#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include <shaderc/shaderc.h>

Shader::Shader(const std::vector<std::string>& files)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, EnumInfo::shadercApiVersion(Graphics::API_VERSION));

	for (auto& file : files)
	{
		std::string path = std::string("data/shaders/") + file;
		std::string source = File::read(path);
	
		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, EnumInfo::shadercType(getShaderType(path)), path.c_str(), options);

		VkShaderModule shader_module = createShaderModule(source);
		shader_modules.push_back(shader_module);

		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = EnumInfo::shaderType(getShaderType(path));
		shader_stage_info.module = shader_module;
		shader_stage_info.pName = "main";
		shader_stage_infos.push_back(shader_stage_info);
	}
}

Shader::Shader(const std::string& file)
	: Shader(std::vector<std::string>{ file })
{
}

Shader::~Shader()
{
	for(auto& shader_module : shader_modules)
		vkDestroyShaderModule(*Graphics::logical_device, shader_module, nullptr);//I think cleanup should be much further than this
}

VkShaderModule Shader::createShaderModule(const std::string& source) const
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = source.size() / sizeof(uint32_t);
	create_info.pCode = (const uint32_t*)source.data();

	VkShaderModule shader_module;
	Graphics::vulkanAssert(vkCreateShaderModule(*Graphics::logical_device, &create_info, nullptr, &shader_module));

	return shader_module;

}

ShaderType Shader::getShaderType(const std::string& file)
{
	if (file.find(".vert") != std::string::npos)
		return ShaderType::VERTEX;

	if (file.find(".frag") != std::string::npos)
		return ShaderType::FRAGMENT;

	if (file.find(".geom") != std::string::npos)
		return ShaderType::GEOMETRY;

	if (file.find(".comp") != std::string::npos)
		return ShaderType::COMPUTE;

	if (file.find(".tesc") != std::string::npos)
		return ShaderType::TESSELATION_CONTROL;

	if (file.find(".tese") != std::string::npos)
		return ShaderType::TESSELATION_EVALUATION;

	SK_ERROR("Couldn't determine shader type from file extension, try using these extensions: .vert .frag .tesc .tese .comp .geom");
	return ShaderType::NONE;
}
