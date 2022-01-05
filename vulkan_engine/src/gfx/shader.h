#pragma once

enum class ShaderType
{
	NONE,
	VERTEX,
	FRAGMENT,
	GEOMETRY,
	COMPUTE,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION
};

class Shader
{
public:
	Shader(const std::vector<std::string>& files);
	~Shader();

	const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageInfos() const { return shader_stage_infos; }

private:
	VkShaderModule createShaderModule(const std::vector<char>& source) const;
	ShaderType getShaderType(const std::string& file);

private:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<VkShaderModule> shader_modules;
};