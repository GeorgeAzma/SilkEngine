#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

Shader::Shader(const std::string& file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines);
}

void Shader::compile(const std::vector<Define>& defines)
{
	for (auto& shader_module : shader_modules)
		vkDestroyShaderModule(*Graphics::logical_device, shader_module, nullptr);
	shader_modules.clear();

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, EnumInfo::shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.SetIncluder(makeUnique<ShaderIncluder>());
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
#ifdef SK_ENABLE_DEBUG_OUTPUT
	options.SetGenerateDebugInfo();
#endif
	for(auto& define : defines)
		options.AddMacroDefinition(define.name, define.value);

	std::string path = std::string("data/shaders/") + file + ".glsl";
	std::string cache_path = std::string("data/cache/shaders/") + file + ".glsl";
	std::unordered_map<uint32_t, std::string> shader_sources = parse(path);
	std::unordered_map<uint32_t, std::vector<uint32_t>> shader_binaries;

	for (auto&& [type, source] : shader_sources)
	{
		std::string file_cache_path = cache_path + EnumInfo::shaderTypeFileExtension((ShaderType)type) + ".spv";
		std::ifstream in(file_cache_path, std::ios::ate | std::ios::binary);
#ifndef SK_ENABLE_DEBUG_OUTPUT
		if (in.is_open())
		{
			size_t size = in.tellg();
			in.seekg(0);
			shader_binaries[(uint32_t)type].resize(size / sizeof(uint32_t));
			in.read((char*)shader_binaries[(uint32_t)type].data(), size);

			SK_TRACE("Shader cache loaded: {0}", file_cache_path);
}
		else
#endif
		{
			shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(source, EnumInfo::shadercType((ShaderType)type), path.c_str(), options);
			SK_ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success, result.GetErrorMessage());
			source = { result.cbegin(), result.cend() };

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

	for (auto&& [type, data] : shader_binaries)
		reflect(shader_binaries[type]);

	SK_TRACE("Shader loaded: {0}", path);
}

Shader::~Shader()
{
	for(auto& shader_module : shader_modules)
		vkDestroyShaderModule(*Graphics::logical_device, shader_module, nullptr);
}

std::unordered_map<uint32_t, std::string> Shader::parse(const std::string& file)
{
	std::unordered_map<uint32_t, std::string> shader_sources;

	std::string source = File::read(file);

	constexpr char type_token[] = "#type";
	size_t type_token_length = strlen(type_token);
	size_t pos = source.find(type_token, 0);
	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + type_token_length + 1;
		std::string type_string = source.substr(begin, eol - begin);
		SK_ASSERT(EnumInfo::shaderString(type_string) != ShaderType::NONE, "Invalid shader type specified");

		size_t next_line_pos = source.find_first_not_of("\r\n", eol);
		SK_ASSERT(next_line_pos != std::string::npos, "Syntax error");
		pos = source.find(type_token, next_line_pos);

		shader_sources[(uint32_t)EnumInfo::shaderString(type_string)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
	}

	return shader_sources;
}

void Shader::reflect(const std::vector<uint32_t>& source)
{
//	std::vector<uint32_t> s = source;
//	spirv_cross::Compiler compiler(std::move(s));
//	
//	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
//	
//	SK_TRACE("VulkanShader::Reflect: ");
//	SK_TRACE("	{0} uniform buffers", resources.uniform_buffers.size());
//	SK_TRACE("	{0} resources", resources.sampled_images.size());
//
//	SK_TRACE("Uniform buffers:");
//	for (const auto& resource : resources.uniform_buffers)
//	{
//		const auto& buffer_type = compiler.get_type(resource.base_type_id);
//		uint32_t buffer_size = compiler.get_declared_struct_size(buffer_type);
//		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
//		int member_count = buffer_type.member_types.size();
//
//		SK_TRACE("  {0}", resource.name);
//		SK_TRACE("    Size = {0}", buffer_size);
//		SK_TRACE("    Binding = {0}", binding);
//		SK_TRACE("    Members = {0}", member_count);
//	}
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
