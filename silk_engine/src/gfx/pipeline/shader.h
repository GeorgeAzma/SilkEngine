#pragma once

#include "gfx/enums.h"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

class Shader : NonCopyable
{
    struct Define
    {
        std::string name = "";
        std::string value = "";
    };

    struct Extension
    {
        std::string name = "";
        std::string behavior = "require";
    };

    class Includer : public glslang::TShader::Includer
    {
        IncludeResult* includeLocal(const char* header, const char* includer, size_t inclusion_depth) override;
        IncludeResult* includeSystem(const char* header, const char* includer, size_t inclusion_depth) override;
        void releaseInclude(IncludeResult* result) override;
    };

public:
    Shader(const std::string& file, const std::vector<Define>& defines = {});
	~Shader();

    void compile(const std::vector<Define>& defines = {});

	const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageInfos() const { return shader_stage_infos; }
	const std::vector<VkShaderModule>& getShaderModules() const { return shader_modules; }
	operator const VkShaderModule& () const { return shader_modules[0]; }

public:
    static EShLanguage getEshLanguage(ShaderType stage_flag);
    static VkShaderStageFlagBits getVulkanType(ShaderType shader_type);
    static ShaderType getStringType(const std::string& shader_string);
    static std::string getTypeFileExtension(ShaderType shader_type);
    static glslang::EShTargetClientVersion getEshClientVersion(APIVersion api_version);
    static TBuiltInResource getResources();

private:
	std::unordered_map<uint32_t, std::string> parse(const std::string& file);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& source) const;
    
private:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<VkShaderModule> shader_modules;
    std::string file;
};