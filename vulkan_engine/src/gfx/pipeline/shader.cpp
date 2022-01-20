#include "shader.h"
#include "utils/file_utils.h"
#include "gfx/graphics.h"

Shader::Shader(const std::vector<std::string>& files)
{
	for (auto& file : files)
	{
		std::vector<char> source = FileUtils::read(file);
		VkShaderModule shader_module = createShaderModule(source);
		shader_modules.push_back(shader_module);

		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = EnumInfo::shaderType(getShaderType(file));
		shader_stage_info.module = shader_module;
		shader_stage_info.pName = "main";
		shader_stage_infos.push_back(shader_stage_info);
	}
}

Shader::~Shader()
{
	for(auto& shader_module : shader_modules)
		vkDestroyShaderModule(*Graphics::logical_device, shader_module, nullptr);//I think cleanup should be much further than this
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& source) const
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = source.size();
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

	if (file.find(".tcs") != std::string::npos)
		return ShaderType::TESSELATION_CONTROL;

	if (file.find(".tes") != std::string::npos)
		return ShaderType::TESSELATION_EVALUATION;

	VE_ERROR("Couldn't determine shader type from file extension, try using these extensions: .vert .frag .tcs .tes");
	return ShaderType::NONE;
}
