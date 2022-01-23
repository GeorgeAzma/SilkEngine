#pragma once
#include "gfx/enums.h"

class Shader : NonCopyable
{
public:
	Shader(const std::vector<std::string>& files);
	Shader(const std::string& file); //Handy for compute shader
	~Shader();

	const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageInfos() const { return shader_stage_infos; }

	operator const VkShaderModule& () const { return shader_modules[0]; }
	const std::vector<VkShaderModule>& getShaderModules() const { return shader_modules; }

private:
	VkShaderModule createShaderModule(const std::vector<char>& source) const;
	static ShaderType getShaderType(const std::string& file);

private:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<VkShaderModule> shader_modules;
};