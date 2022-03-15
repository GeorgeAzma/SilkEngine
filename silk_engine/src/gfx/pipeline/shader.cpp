#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "scene/resources.h"
#include "utils/string.h"
#include "gfx/renderer.h"
#include <spirv_cross/spirv_cross.hpp>
 
shaderc_include_result* Shader::Includer::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
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
}

void Shader::Includer::ReleaseInclude(shaderc_include_result* data)
{
	delete static_cast<std::array<std::string, 2>*>(data->user_data);
	delete data;
}

Shader::Shader(const std::filesystem::path& file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines);
}

void Shader::compile(const std::vector<Define>& defines)
{
	bool resources_was_compiled = resources.size();
	for (const auto& stage : stages)
		Graphics::logical_device->destroyShaderModule(stage.module);
	stages.clear();

	std::string defines_str = "";
	for (const auto& define : defines)
		defines_str += define.name + define.value;

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.AddMacroDefinition("MAX_IMAGE_SLOTS", std::to_string(Graphics::MAX_IMAGE_SLOTS));
	options.AddMacroDefinition("MAX_LIGHTS", std::to_string(Renderer::MAX_LIGHTS));
	options.AddMacroDefinition("DIFFUSE_TEXTURE", "0");
	options.AddMacroDefinition("NORMAL_TEXTURE", "1");
	options.AddMacroDefinition("AO_TEXTURE", "2");
	options.AddMacroDefinition("HEIGHT_TEXTURE", "3");
	options.AddMacroDefinition("SPECULAR_TEXTURE", "4");
	options.AddMacroDefinition("EMMISIVE_TEXTURE", "5");
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

	for (auto&& [type, source] : shader_sources)
	{
		stages.emplace_back();
		auto& stage = stages.back();
		stage.stage = getVulkanType((Type)type);

		std::string file_cache_path = cache_path.string() + getTypeFileExtension((Type)type) + ".spv";

		std::ifstream in(file_cache_path, std::ios::ate | std::ios::binary);
		if (in.is_open())
		{
			size_t size = in.tellg();
			in.seekg(0);
			stage.binary.resize(size / sizeof(uint32_t));
			in.read((char*)stage.binary.data(), size);

			SK_TRACE("Shader cache loaded: {0}", file_cache_path);
		}
		else
		{
			auto preprocess_result = compiler.PreprocessGlsl(source, shadercType((Type)type), path.string().c_str(), options);
			SK_ASSERT(preprocess_result.GetCompilationStatus() == shaderc_compilation_status_success, preprocess_result.GetErrorMessage());
			std::string preprocessed_source(preprocess_result.begin());

			shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(preprocessed_source, shadercType((Type)type), path.string().c_str(), options);
			SK_ASSERT(compilation_result.GetCompilationStatus() == shaderc_compilation_status_success, compilation_result.GetErrorMessage());
			stage.binary = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend());

			std::ofstream out(file_cache_path, std::ios::binary | std::ios::trunc);
			SK_ASSERT(out.is_open(), "Couldn't create shader cache file: {0}", file_cache_path);
			out.write((const char*)stage.binary.data(), stage.binary.size() * sizeof(uint32_t));
		
			SK_TRACE("Shader cache created: {0}", file_cache_path);
		}  

		vk::ShaderModuleCreateInfo ci{};
		ci.codeSize = stage.binary.size() * sizeof(uint32_t);
		ci.pCode = stage.binary.data();
		stage.module = Graphics::logical_device->createShaderModule(ci);
	} 
	
#define SK_WEIRD_ERROR_FIX 1 //If you have error in this file, set this to 0, compile (you will get an error), set it back to 1, recompile (IDK why this works, help), if it still doesn't work retry couple times
#if SK_WEIRD_ERROR_FIX
	if (!resources_was_compiled)
	{
		for (const auto& stage : stages)
		{
			const auto& binary = stage.binary;
			auto stage_flag = stage.stage;
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

			auto constants = spirv_compiler.get_specialization_constants();
			for (auto& constant : constants)
			{
				auto& shader_constant = this->constants[spirv_compiler.get_name(constant.id)];
				shader_constant = { constant.constant_id, shader_constant.stage | stage_flag };
			}

			//Local size reflection
			local_size = glm::uvec3(0);
			local_size[0] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
			local_size[1] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
			local_size[2] = spirv_compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
			if (local_size != glm::uvec3(0))
				local_size = glm::max(local_size, glm::uvec3(1));
		}

		std::unordered_map<uint32_t, uint32_t> write_indices;
		for (const auto& resource : this->resources)
			write_indices[resource.set] = 0;
		for (const auto& resource : this->resources)
		{
			auto& write_index = write_indices[resource.set];
			auto& set = descriptor_sets.at(resource.set);
			set->add(resource.binding, resource.count, resource.type, resource.stage);
			resource_locations.emplace(resource.name, ResourceLocation{ resource.set, write_index });
			++write_index;
		}
	
		for (auto& descriptor_set : descriptor_sets)
			descriptor_set.second->build();
	}
#endif
#undef SK_WEIRD_ERROR_FIX

	SK_TRACE("Shader loaded: {0}", path);
}

void Shader::set(const std::string& resource_name, const std::vector<vk::DescriptorBufferInfo>& buffer_infos)
{
	const auto& resource_location = resource_locations.at(resource_name);
	descriptor_sets.at(resource_location.set)->setBufferInfo(resource_location.write_index, buffer_infos);
}

void Shader::set(const std::string& resource_name, const std::vector<vk::DescriptorImageInfo>& image_infos)
{
	const auto& resource_location = resource_locations.at(resource_name);
	descriptor_sets.at(resource_location.set)->setImageInfo(resource_location.write_index, image_infos);
}

const Shader::ResourceLocation* Shader::getIfExists(const std::string& resource_name) const
{
	auto resource_location = resource_locations.find(resource_name);
	return (resource_location != resource_locations.end()) ? &resource_location->second : nullptr;
}

void Shader::bindDescriptors()
{
	for (auto&& [set, descriptor_set] : descriptor_sets)
		descriptor_set->bind(set);
}
  
Shader::~Shader()
{
	for (const auto& stage : stages)
		Graphics::logical_device->destroyShaderModule(stage.module);
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

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage, vk::DescriptorType type)
{
	for (Resource& resource : this->resources)
	{
		if (resource.name == spirv_resource.name)
		{
			resource.stage |= stage;
			return;
		}
	}

	const spirv_cross::SPIRType& spir_type = compiler.get_type(spirv_resource.type_id);
	uint32_t set = compiler.get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
	uint32_t binding = compiler.get_decoration(spirv_resource.id, spv::DecorationBinding);

	if (descriptor_sets.find(set) == descriptor_sets.end())
		descriptor_sets.emplace(set, makeShared<DescriptorSet>());

	uint32_t count = 0;
	for (const auto& arr : spir_type.array)
		count += arr;
	if (!count)
		count = 1;

	Resource resource{};
	resource.id = (size_t)spir_type.basetype;
	resource.count = count;
	resource.set = set;
	resource.binding = binding;
	resource.stage = stage;
	resource.type = type;
	resource.name = spirv_resource.name;
	this->resources.emplace_back(std::move(resource));
}

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, vk::ShaderStageFlags stage)
{
	auto ranges = compiler.get_active_buffer_ranges(resources.push_constant_buffers.front().id);
	for (auto& range : ranges)
	{
		for (auto& push_constant : push_constants)
		{
			if (push_constant.size == range.range && push_constant.offset == range.offset)
			{
				push_constant.stageFlags |= stage;
				return;
			}
		}
		vk::PushConstantRange push_constant_range{};
		push_constant_range.offset = range.offset;
		push_constant_range.size = range.range;
		push_constant_range.stageFlags = stage;
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