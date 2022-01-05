#include "shader.h"
#include "utils/file_utils.h"
#include "graphics.h"

Shader::Shader(const std::vector<std::string>& files)
{
	//Reason we process multiple shader files at a time 
	//is that later on we might combine shaders into 
	//a one shader file which will process all the 
	//shader stages at once similar to this
	for (auto& file : files)
	{
		ShaderType type = getShaderType(file);

		std::vector<char> source = FileUtils::read(file);
		VkShaderModule shader_module = createShaderModule(source);
		shader_modules.push_back(shader_module);

		VkShaderStageFlagBits stage_flag{};

		switch (type)
		{
		case ShaderType::VERTEX:
			stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderType::FRAGMENT:
			stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case ShaderType::GEOMETRY:
			stage_flag = VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case ShaderType::COMPUTE:
			stage_flag = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case ShaderType::TESSELATION_CONTROL:
			stage_flag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			break;
		case ShaderType::TESSELATION_EVALUATION:
			stage_flag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			break;
		default:
			VE_CORE_CRITICAL("Shader type not supported, try renaming shader files to: .vert/.frag/.geom/.tcs/.tes");
		}

		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = stage_flag;
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
	if (file.find("vert") != std::string::npos)
		return ShaderType::VERTEX;

	if (file.find("frag") != std::string::npos)
		return ShaderType::FRAGMENT;

	if (file.find("tcs") != std::string::npos)
		return ShaderType::TESSELATION_CONTROL;

	if (file.find("tes") != std::string::npos)
		return ShaderType::TESSELATION_EVALUATION;

	return ShaderType::NONE;
}
