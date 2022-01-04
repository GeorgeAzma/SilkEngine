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

private:
	VkShaderModule createShaderModule(const std::vector<char>& source) const;
	ShaderType getShaderType(const std::string& file);

private:
	const VkDevice* logical_device = nullptr;
};