#pragma once

#include "gfx/enums.h"
#include <shaderc/shaderc.hpp>

class Shader : NonCopyable
{
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
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
	Shader(const std::string& file); //Handy for compute shader
	~Shader();

	const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageInfos() const { return shader_stage_infos; }

	operator const VkShaderModule& () const { return shader_modules[0]; }
	const std::vector<VkShaderModule>& getShaderModules() const { return shader_modules; }

private:
	std::unordered_map<uint32_t, std::string> parse(const std::string& file);
	void reflect(const std::vector<uint32_t>& source);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& source) const;

private:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<VkShaderModule> shader_modules;
};