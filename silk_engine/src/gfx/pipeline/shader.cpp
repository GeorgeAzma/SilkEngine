#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include <shaderc/shaderc.hpp>

Shader::Shader(const std::string& file)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, EnumInfo::shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.SetWarningsAsErrors();
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
#ifdef SK_ENABLE_DEBUG_OUTPUT
	options.SetGenerateDebugInfo();
#endif

	std::string path = std::string("data/shaders/") + file + ".glsl";
	std::string cache_path = std::string("data/cache/shaders/") + file + ".glsl";
	std::unordered_map<uint32_t, std::string> shader_sources = parse(path);
	std::unordered_map<uint32_t, std::vector<uint32_t>> shader_binaries;

	for (auto&& [type, source] : shader_sources)
	{
		std::string file_cache_path = cache_path + EnumInfo::shaderTypeFileExtension((ShaderType)type) + ".spv";
		std::ifstream in(file_cache_path, std::ios::ate | std::ios::binary);
		if (in.is_open())
		{
			size_t size = in.tellg();
			in.seekg(0);
			shader_binaries[(uint32_t)type].resize(size / sizeof(uint32_t));
			in.read((char*)shader_binaries[(uint32_t)type].data(), size);

			SK_TRACE("Shader cache loaded: {0}", file_cache_path);
		}
		else
		{
			shaderc::SpvCompilationResult compiled_shader = compiler.CompileGlslToSpv(source, EnumInfo::shadercType((ShaderType)type), path.c_str(), options);
			SK_ASSERT(compiled_shader.GetCompilationStatus() == shaderc_compilation_status_success, compiled_shader.GetErrorMessage());

			auto& shader_binary = shader_binaries[type];
			shader_binary = std::vector<uint32_t>(compiled_shader.cbegin(), compiled_shader.cend());

			std::ofstream out(file_cache_path, std::ios::binary);
			SK_ASSERT(out.is_open(), "Couldn't create shader cache file: {0}", file_cache_path);
			out.write((char*)shader_binary.data(), shader_binary.size() * sizeof(uint32_t));

			SK_TRACE("Shader cache created: {0}", file_cache_path);
		}

		VkShaderModule shader_module = createShaderModule(shader_binaries[type]);
		shader_modules.push_back(shader_module);

		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = EnumInfo::shaderType((ShaderType)type);
		shader_stage_info.module = shader_module;
		shader_stage_info.pName = "main";
		shader_stage_infos.push_back(shader_stage_info);
	}

	SK_TRACE("Shader loaded: {0}", path);
}

Shader::~Shader()
{
	for(auto& shader_module : shader_modules)
		vkDestroyShaderModule(*Graphics::logical_device, shader_module, nullptr);//I think cleanup should be much further than this
}

std::unordered_map<uint32_t, std::string> Shader::parse(const std::string& file)
{
	std::unordered_map<uint32_t, std::string> shader_sources;

	std::string source = File::read(file);

	const char* type_token = "#type";
	size_t type_token_length = strlen(type_token);
	size_t pos = source.find(type_token, 0); //Start of shader type declaration line
	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + type_token_length + 1; //Start of shader type name (after "#type " keyword)
		std::string type_string = source.substr(begin, eol - begin);
		SK_ASSERT(EnumInfo::shaderString(type_string) != ShaderType::NONE, "Invalid shader type specified");

		size_t next_line_pos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
		SK_ASSERT(next_line_pos != std::string::npos, "Syntax error");
		pos = source.find(type_token, next_line_pos); //Start of next shader type declaration line

		shader_sources[(uint32_t)EnumInfo::shaderString(type_string)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
	}

	return shader_sources;
}

VkShaderModule Shader::createShaderModule(const std::vector<uint32_t>& source) const
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = source.size() * sizeof(uint32_t);
	create_info.pCode = source.data();

	VkShaderModule shader_module;
	Graphics::vulkanAssert(vkCreateShaderModule(*Graphics::logical_device, &create_info, nullptr, &shader_module));

	return shader_module;

}
