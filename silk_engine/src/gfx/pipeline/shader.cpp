#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "scene/resources.h"
#include "utils/string.h"
#include <spirv_cross/spirv_reflect.hpp>

Shader::Shader(const std::filesystem::path& file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines);
}

void Shader::compile(const std::vector<Define>& defines)
{
	bool resources_was_compiled = resources.size();
	for (auto& pipeline_shader_stage_info : pipeline_shader_stage_infos)
		Graphics::logical_device->destroyShaderModule(pipeline_shader_stage_info.module);
	pipeline_shader_stage_infos.clear();

	std::string defines_str = "";
	for (const auto& define : defines)
		defines_str += define.name + define.value;

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.AddMacroDefinition("MAX_IMAGE_SLOTS", std::to_string(Graphics::MAX_IMAGE_SLOTS));
	options.SetIncluder(std::make_unique<Includer>());
#ifdef SK_ENABLE_DEBUG_OUTPUT
	options.SetGenerateDebugInfo();
#endif
	for (auto& define : defines)
		options.AddMacroDefinition(define.name, define.value);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	std::filesystem::path path = (std::filesystem::path("data/shaders") / file).string() + ".glsl";
	std::filesystem::path cache_path = (std::filesystem::path("data/cache/shaders") / file).string() + defines_str + ".glsl";
	std::unordered_map<uint32_t, std::string> shader_sources = parse(path);
	std::unordered_map<uint32_t, std::vector<uint32_t>> shader_binaries;

	for (auto&& [type, source] : shader_sources)
	{
		std::string file_cache_path = cache_path.string() + getTypeFileExtension((Type)type) + ".spv";
//#ifndef SK_ENABLE_DEBUG_OUTPUT
		std::ifstream in(file_cache_path, std::ios::ate | std::ios::binary);
		if (in.is_open())
		{
			size_t size = in.tellg();
			in.seekg(0);
			shader_binaries[type].resize(size / sizeof(uint32_t));
			in.read((char*)shader_binaries[type].data(), size);

			SK_TRACE("Shader cache loaded: {0}", file_cache_path);
		}
		else
//#endif
		{
			auto preprocess_result = compiler.PreprocessGlsl(source, shadercType((Type)type), path.string().c_str(), options);
			SK_ASSERT(preprocess_result.GetCompilationStatus() == shaderc_compilation_status_success, preprocess_result.GetErrorMessage());
			std::string preprocessed_source(preprocess_result.begin());

			shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(preprocessed_source, shadercType((Type)type), path.string().c_str(), options);
			SK_ASSERT(compilation_result.GetCompilationStatus() == shaderc_compilation_status_success, compilation_result.GetErrorMessage());
			shader_binaries[type] = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend());

			std::ofstream out(file_cache_path, std::ios::binary | std::ios::trunc);
			SK_ASSERT(out.is_open(), "Couldn't create shader cache file: {0}", file_cache_path);
			out.write((const char*)shader_binaries[type].data(), shader_binaries[type].size() * sizeof(uint32_t));
		
			SK_TRACE("Shader cache created: {0}", file_cache_path);
		}
		
		vk::PipelineShaderStageCreateInfo pipeline_shader_stage_info{};
		pipeline_shader_stage_info.stage = getVulkanType((Type)type);
		pipeline_shader_stage_info.module = createShaderModule(shader_binaries[type]);
		pipeline_shader_stage_info.pName = "main";
		

		pipeline_shader_stage_infos.emplace_back(std::move(pipeline_shader_stage_info));
	}

#define SK_WEIRD_ERROR_FIX 1 //If you have error in this file, set this to 0, compile (you will get an error), set it back to 1, recompile (IDK why this works, help), if it still doesn't work retry couple times
#if SK_WEIRD_ERROR_FIX
	if (!resources_was_compiled)
	{
		for (auto&& [type, binary] : shader_binaries)
		{
			auto stage_flag = getVulkanType((Type)type);
			spirv_cross::Compiler spirv_compiler(binary);
			spirv_cross::ShaderResources shader_resources = spirv_compiler.get_shader_resources();

			for (const spirv_cross::Resource& sampled_image : shader_resources.sampled_images)
				loadResource(sampled_image, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eCombinedImageSampler);

			for (const spirv_cross::Resource& seperate_image : shader_resources.separate_images)
				loadResource(seperate_image, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eSampledImage);

			for (const spirv_cross::Resource& seperate_sampler : shader_resources.separate_samplers)
				loadResource(seperate_sampler, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eSampler);

			for (const spirv_cross::Resource& storage_buffer : shader_resources.storage_buffers)
				loadResource(storage_buffer, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eStorageBuffer);

			for (const spirv_cross::Resource& storage_image : shader_resources.storage_images)
				loadResource(storage_image, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eStorageImage);

			for (const spirv_cross::Resource& subpass_input : shader_resources.subpass_inputs)
				loadResource(subpass_input, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eInputAttachment);

			for (const spirv_cross::Resource& uniform_buffer : shader_resources.uniform_buffers)
				loadResource(uniform_buffer, spirv_compiler, shader_resources, stage_flag, vk::DescriptorType::eUniformBuffer);

			for (const spirv_cross::Resource& push_constant : shader_resources.push_constant_buffers)
				loadPushConstant(push_constant, spirv_compiler, shader_resources, stage_flag);

			//Local size reflection
			local_size = glm::uvec3(0);
			local_size[0] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
			local_size[1] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
			local_size[2] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
			if (local_size != glm::uvec3(0))
				local_size = glm::max(local_size, glm::uvec3(1));
		}

		for (const auto& resource : this->resources)
			descriptor_sets.at(resource.set)->add(resource.binding, resource.count, resource.type, resource.stage_flags);
	
		for (auto& descriptor_set : descriptor_sets)
			descriptor_set.second->build();
	}
#endif
#undef SK_WEIRD_ERROR_FIX

	SK_TRACE("Shader loaded: {0}", path);
}

Shader::~Shader()
{
	for (auto& pipeline_shader_stage_info : pipeline_shader_stage_infos)
		Graphics::logical_device->destroyShaderModule(pipeline_shader_stage_info.module);
}

std::unordered_map<uint32_t, std::string> Shader::parse(const std::filesystem::path& file)
{
	std::unordered_map<uint32_t, std::string> shader_sources;

	std::string source = File::read(file);

	constexpr const char* type_token = "#type";
	size_t type_token_length = strlen(type_token);
	size_t pos = source.find(type_token, 0);
	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + type_token_length + 1;
		std::string type_string = source.substr(begin, eol - begin);

		size_t next_line_pos = source.find_first_not_of("\r\n", eol);
		SK_ASSERT(next_line_pos != std::string::npos, "Syntax error");
		pos = source.find(type_token, next_line_pos);

		shader_sources[(uint32_t)getStringType(type_string)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
	}

	return shader_sources;
}

vk::ShaderModule Shader::createShaderModule(const std::vector<uint32_t>& source) const
{
	vk::ShaderModuleCreateInfo ci{};
	ci.codeSize = source.size() * sizeof(uint32_t);
	ci.pCode = source.data();

	vk::ShaderModule shader_module = Graphics::logical_device->createShaderModule(ci);

	return shader_module;
}

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage_flag, vk::DescriptorType type)
{
	const spirv_cross::SPIRType& spir_type = compiler.get_type(spirv_resource.type_id);

	for (Resource& resource : this->resources)
	{
		if (resource.id == spir_type.basetype)
		{
			resource.stage_flags |= stage_flag;
			return;
		}
	}
	
	uint32_t set = compiler.get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
	uint32_t binding = compiler.get_decoration(spirv_resource.id, spv::DecorationBinding);
	
	if (descriptor_sets.find(set) == descriptor_sets.end())
		descriptor_sets.emplace(set, makeShared<DescriptorSet>());
	
	uint32_t count = 0;
	for (const auto& arr : spir_type.array)
		count += arr;
	if (!count)
		count = 1;

	Resource resource
	{
		.id = (size_t)spir_type.basetype,
		.count = count,
		.set = set,
		.binding = binding,
		.stage_flags = stage_flag,
		.type = type
	};
	this->resources.emplace_back(std::move(resource));
}

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage_flag)
{
	auto ranges = compiler.get_active_buffer_ranges(resources.push_constant_buffers.front().id);
	for (auto& range : ranges)
	{
		for (auto& push_constant : push_constants)
		{
			if (push_constant.size == range.range && push_constant.offset == range.offset)
			{
				push_constant.stageFlags |= stage_flag;
				return;
			}
		}
		vk::PushConstantRange push_constant_range{};
		push_constant_range.offset = range.offset;
		push_constant_range.size = range.range;
		push_constant_range.stageFlags = stage_flag;
		push_constants.emplace_back(std::move(push_constant_range));
	}
}

Shader::Type Shader::getStringType(const std::string& shader_string)
{
	if (shader_string == "vertex") return Type::VERTEX;
	else if (shader_string == "fragment") return Type::FRAGMENT;
	else if (shader_string == "geometry") return Type::GEOMETRY;
	else if (shader_string == "compute") return Type::COMPUTE;
	else if (shader_string == "tesselation_control") return Type::TESSELATION_CONTROL;
	else if (shader_string == "tesselation_evaluation") return Type::TESSELATION_EVALUATION;

	SK_ERROR("Unsupported shader type string specified: {0}. try writing #type vertex/fragment/geometry/compute/tesselation_control/tesselation_evaluation", shader_string);
	return Shader::Type(0);
}

shaderc_shader_kind Shader::shadercType(Type shader_type)
{
	switch (shader_type)
	{
	case Shader::Type::NONE:
	case Shader::Type::VERTEX: return shaderc_vertex_shader;
	case Shader::Type::FRAGMENT: return shaderc_fragment_shader;
	case Shader::Type::GEOMETRY: return shaderc_geometry_shader;
	case Shader::Type::COMPUTE: return shaderc_compute_shader;
	case Shader::Type::TESSELATION_CONTROL: return shaderc_tess_control_shader;
	case Shader::Type::TESSELATION_EVALUATION: return shaderc_tess_evaluation_shader;
	}
	
	SK_ERROR("Unsupported shader type specified");
	return shaderc_shader_kind(0);
}

shaderc_env_version Shader::shadercApiVersion(APIVersion api_version)
{
	switch (api_version)	
	{
	case APIVersion::VULKAN_1_0: return shaderc_env_version_vulkan_1_0;
	case APIVersion::VULKAN_1_1: return shaderc_env_version_vulkan_1_1;
	case APIVersion::VULKAN_1_2: return shaderc_env_version_vulkan_1_2;
	}

	SK_ERROR("Unsupported api version specified");
	return shaderc_env_version(0);
}

vk::ShaderStageFlagBits Shader::getVulkanType(Type shader_type)
{
	switch (shader_type)
	{
	case Type::VERTEX: return vk::ShaderStageFlagBits::eVertex;
	case Type::FRAGMENT: return vk::ShaderStageFlagBits::eFragment;
	case Type::GEOMETRY: return vk::ShaderStageFlagBits::eGeometry;
	case Type::COMPUTE: return vk::ShaderStageFlagBits::eCompute;
	case Type::TESSELATION_CONTROL: return vk::ShaderStageFlagBits::eTessellationControl;
	case Type::TESSELATION_EVALUATION: return vk::ShaderStageFlagBits::eTessellationEvaluation;
	}

	SK_ERROR("Unsupported shader type specified: {0}.", shader_type);
	return vk::ShaderStageFlagBits(0);
}

std::string Shader::getTypeFileExtension(Shader::Type shader_type)
{
	switch (shader_type)
	{
	case Type::VERTEX: return ".vert";
	case Type::FRAGMENT: return ".frag";
	case Type::GEOMETRY: return ".geom";
	case Type::COMPUTE: return ".comp";
	case Type::TESSELATION_CONTROL: return ".tesc";
	case Type::TESSELATION_EVALUATION: return ".tese";
	}

	SK_ERROR("Unsupported shader type specified: {0}.", shader_type);
	return "";
}