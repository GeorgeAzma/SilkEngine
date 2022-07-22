#include "shader.h"
#include "io/file.h"
#include "gfx/devices/logical_device.h"
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

Shader::Shader(std::string_view file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines, false);
}

void Shader::compile(const std::vector<Define>& defines, bool force)
{
	for (const auto& stage : stages)
		Graphics::logical_device->destroyShaderModule(stage.module);
	stages.clear();
	resources.clear();
	local_size = glm::vec3(0);
	descriptor_sets.clear();
	push_constants.clear();
	resource_locations.clear();
	constants.clear();

	std::string defines_str = "";
	for (const auto& define : defines)
		defines_str += define.name + define.value;

	shaderc::Compiler compiler{};
	shaderc::CompileOptions options{};
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	options.AddMacroDefinition("MAX_IMAGE_SLOTS", std::to_string(Graphics::MAX_IMAGE_SLOTS));
	options.AddMacroDefinition("MAX_LIGHTS", std::to_string(Renderer::MAX_LIGHTS));
	options.AddMacroDefinition("DIFFUSE_TEXTURE", "0");
	options.AddMacroDefinition("NORMAL_TEXTURE", "1");
	options.AddMacroDefinition("AO_TEXTURE", "2");
	options.AddMacroDefinition("HEIGHT_TEXTURE", "3");
	options.AddMacroDefinition("SPECULAR_TEXTURE", "4");
	options.AddMacroDefinition("EMMISIVE_TEXTURE", "5");
	options.SetIncluder(std::make_unique<Includer>());
	options.SetGenerateDebugInfo();
	//options.SetAutoMapLocations(true);

	for (auto& define : defines)
		options.AddMacroDefinition(define.name, define.value);

	std::filesystem::path path = std::filesystem::path("data/shaders") / (file.string() + ".glsl");
	std::filesystem::path cache_path = std::filesystem::path("data/cache/shaders") / (file.string() + defines_str + ".glsl");
	std::unordered_map<uint32_t, std::string> shader_sources = parse(path);

	for (auto&& [type, source] : shader_sources)
	{
		stages.emplace_back();
		auto& stage = stages.back();
		stage.stage = Type(type);

		std::string file_cache_path = cache_path.string() + getTypeFileExtension((Type)type) + ".spv";

		std::ifstream in(file_cache_path, std::ios::binary);
		bool cache_exists = in.is_open();

		// Read cache
		if (cache_exists)
		{
			if (force)
			{
				in.close();
				std::filesystem::remove(file_cache_path);
			}
			else
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				stage.binary.resize(size / sizeof(uint32_t));
				in.seekg(0);
				in.read((char*)stage.binary.data(), size);
				SK_TRACE("Shader cache loaded: {0}", file_cache_path);
			}
		}
		in.close();

		// Create cache
		if (!cache_exists || force)
		{
			auto preprocess_result = compiler.PreprocessGlsl(source, shadercType((Type)type), path.string().c_str(), options);
			SK_ASSERT(preprocess_result.GetCompilationStatus() == shaderc_compilation_status_success, preprocess_result.GetErrorMessage());
			std::string preprocessed_source(preprocess_result.begin());

			shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(preprocessed_source, shadercType((Type)type), path.string().c_str(), options); // IF YOU HAVE ERROR HERE SCROLL BELOW, TILL YOU SEE #SK_ERROR_FIX DEFINE
			SK_ASSERT(compilation_result.GetCompilationStatus() == shaderc_compilation_status_success, "{}. SOURCE:\n{}", compilation_result.GetErrorMessage(), preprocessed_source);
			stage.binary = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend());

			std::ofstream out(file_cache_path, std::ios::binary);
			SK_ASSERT(out.is_open(), "Couldn't create shader cache file: {}", file_cache_path);
			out.write((const char*)stage.binary.data(), stage.binary.size() * sizeof(uint32_t));
			out.close();

			SK_TRACE("Shader cache created: {}", file_cache_path);
		}

		VkShaderModuleCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ci.codeSize = stage.binary.size() * sizeof(uint32_t);
		ci.pCode = stage.binary.data();
		stage.module = Graphics::logical_device->createShaderModule(ci);
	} 

//TO FIX THE ERROR SET THIS TO 0, LAUNCH, SET IT BACK TO 1, LAUNCH AGAIN AND IT SHOULD BE FIXED.
#define SK_ERROR_FIX 1
#if SK_ERROR_FIX
	for (const auto& stage : stages)
	{
		spirv_cross::Compiler compiler(stage.binary);
		spirv_cross::ShaderResources shader_resources = compiler.get_shader_resources();
		auto vulkan_stage = toVulkanType(stage.stage);

		for (const spirv_cross::Resource& sampled_image : shader_resources.sampled_images)
			loadResource(sampled_image, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		for (const spirv_cross::Resource& seperate_image : shader_resources.separate_images)
			loadResource(seperate_image, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

		for (const spirv_cross::Resource& seperate_sampler : shader_resources.separate_samplers)
			loadResource(seperate_sampler, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_SAMPLER);

		for (const spirv_cross::Resource& storage_buffer : shader_resources.storage_buffers)
			loadResource(storage_buffer, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		for (const spirv_cross::Resource& storage_image : shader_resources.storage_images)
			loadResource(storage_image, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		for (const spirv_cross::Resource& subpass_input : shader_resources.subpass_inputs)
			loadResource(subpass_input, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

		for (const spirv_cross::Resource& uniform_buffer : shader_resources.uniform_buffers)
			loadResource(uniform_buffer, compiler, shader_resources, vulkan_stage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		for (const spirv_cross::Resource& push_constant : shader_resources.push_constant_buffers)
			loadPushConstant(push_constant, compiler, shader_resources, vulkan_stage);
		
		if(stage.stage == Type::VERTEX)
		{
			constexpr GpuType lut[8][4]
			{
				{ GpuType::FLOAT, GpuType::VEC2, GpuType::VEC3, GpuType::VEC4 },
				{ GpuType::INT, GpuType::IVEC2, GpuType::IVEC3, GpuType::IVEC4 },
				{ GpuType::UINT, GpuType::UVEC2, GpuType::UVEC3, GpuType::UVEC4 },
				{ GpuType::DOUBLE, GpuType::DVEC2, GpuType::DVEC3, GpuType::DVEC4 },
				{ GpuType::UBYTE, GpuType::UVEC2, GpuType::UVEC3, GpuType::UVEC4 },
				{ GpuType::BYTE, GpuType::UVEC2, GpuType::UVEC3, GpuType::UVEC4 },
				{ GpuType::USHORT, GpuType::UVEC2, GpuType::UVEC3, GpuType::UVEC4 },
				{ GpuType::SHORT, GpuType::UVEC2, GpuType::UVEC3, GpuType::UVEC4 }
			};
			constexpr GpuType mat_lut[8][4]
			{
				{ GpuType::FLOAT, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::INT, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::UINT, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::DOUBLE, GpuType::DMAT2, GpuType::DMAT3, GpuType::DMAT4 },
				{ GpuType::UBYTE, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::BYTE, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::USHORT, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
				{ GpuType::SHORT, GpuType::MAT2, GpuType::MAT3, GpuType::MAT4 },
			};

			uint32_t max_location = 0;
			for (auto& input : shader_resources.stage_inputs)
			{
				uint32_t location = compiler.get_decoration(input.id, spv::Decoration::DecorationLocation);
				max_location = std::max(max_location, location + 1);
			}

			std::vector<std::optional<BufferElement>> buffer_elements;
			buffer_elements.resize(max_location, std::nullopt);

			for (auto& input : shader_resources.stage_inputs)
			{
				uint32_t location = compiler.get_decoration(input.id, spv::Decoration::DecorationLocation);
				using Type = spirv_cross::SPIRType::BaseType;
				auto type = compiler.get_type(input.base_type_id);
				size_t type_index = 0;
				switch (type.basetype)
				{
				case Type::Float:
					type_index = 0;
					break;
				case Type::Int:
					type_index = 1;
					break;
				case Type::UInt:
					type_index = 2;
					break;
				case Type::Double:
					type_index = 3;
					break;
				case Type::UByte:
					type_index = 4;
					break;
				case Type::SByte:
					type_index = 5;
					break;
				case Type::UShort:
					type_index = 6;
					break;
				case Type::Short:
					type_index = 7;
					break;
				}

				BufferElement element{};
				element.type = lut[type_index][type.vecsize - 1];

				if (type.vecsize > 1 && type.vecsize == type.columns)
					element.type = mat_lut[type_index][type.columns - 1];

				std::string_view token = "I_";
				if (input.name.size() >= token.size())
				{
					std::string_view str(input.name.begin(), input.name.begin() + token.size());
					element.instanced = (str == token);
				}
				else element.instanced = false;

				buffer_elements[location] = element;
			}

			std::vector<BufferElement> aligned_buffer_elements;
			for (const auto& e : buffer_elements)
			{
				if (e.has_value())
					aligned_buffer_elements.emplace_back(*e);
			}

			buffer_layout = BufferLayout(aligned_buffer_elements);
		}

		auto constants = compiler.get_specialization_constants();
		for (auto& constant : constants)
		{
			auto& shader_constant = this->constants[compiler.get_name(constant.id)];
			shader_constant = { constant.constant_id, shader_constant.stage | vulkan_stage };
		}

		//Local size reflection
		local_size = glm::uvec3(0);
		local_size[0] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
		local_size[1] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
		local_size[2] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
		if (local_size != glm::uvec3(0))
			local_size = glm::max(local_size, glm::uvec3(1));
	}

	for (const auto& resource : this->resources)
	{
		auto& set = descriptor_sets.at(resource.set);
		set->add(resource.binding, resource.count, resource.type, resource.stage);
		resource_locations.emplace(resource.name, ResourceLocation{ resource.set, resource.binding });
	}

	for (auto& descriptor_set : descriptor_sets)
		descriptor_set.second->build();
#endif

	SK_TRACE("Shader loaded: {0}", path);
}

void Shader::reflect()
{
	
}

void Shader::set(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos)
{
	const auto& resource_location = resource_locations.at(resource_name);
	descriptor_sets.at(resource_location.set)->setBufferInfo(resource_location.binding, buffer_infos);
}

void Shader::set(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos)
{
	const auto& resource_location = resource_locations.at(resource_name);
	descriptor_sets.at(resource_location.set)->setImageInfo(resource_location.binding, image_infos);
}

void Shader::setIfExists(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos)
{
	auto resource_location = resource_locations.find(resource_name);
	if(resource_location != resource_locations.end())
		descriptor_sets.at(resource_location->second.set)->setBufferInfo(resource_location->second.binding, buffer_infos);
}

void Shader::setIfExists(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos)
{
	auto resource_location = resource_locations.find(resource_name);
	if (resource_location != resource_locations.end())
		descriptor_sets.at(resource_location->second.set)->setImageInfo(resource_location->second.binding, image_infos);
}

const Shader::ResourceLocation* Shader::getIfExists(std::string_view resource_name) const
{
	auto resource_location = resource_locations.find(resource_name);
	return (resource_location != resource_locations.end()) ? &resource_location->second : nullptr;
}

void Shader::bindDescriptorSets()
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

	const std::string source = File::read(file);

	//Devide shader codes
	std::string token = "#type";
	size_t pos = source.find(token, 0);
	SK_ASSERT(pos != std::string::npos, "Couldn't find #type preprocessor");
	std::string code_source = source.substr(pos);
	std::string param_source = source.substr(0, pos);
	pos = 0;
	while (pos != std::string::npos)
	{
		size_t eol = code_source.find_first_of("\r\n", pos);
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + token.size() + 1;
		std::string value = code_source.substr(begin, eol - begin);

		size_t next_line_pos = code_source.find_first_not_of("\r\n", eol);
		SK_ASSERT(next_line_pos != std::string::npos, "Syntax error");
		pos = code_source.find(token, next_line_pos);

		Type type = getStringType(value);
		shader_sources[(uint32_t)type] = (pos == std::string::npos) ? code_source.substr(next_line_pos) : code_source.substr(next_line_pos, pos - next_line_pos);
	}

	if (!param_source.size()) 
		return shader_sources;

	//Update parameters
	constexpr auto tokens = makeArray("depth_test", "depth_write", "stencil_test", "blend", "sample_shading", "primitive_restart", "rasterizer_discard", "depth_clamp", "depth_bias", "color_blend_logic_op");
	parameters = {};

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		updateParameter(param_source, parameters.enabled[i], tokens[i],
			{
				{ "true", true },
				{ "on", true },
				{ "1", true },
				{ "false", false },
				{ "off", false },
				{ "0", false }
			});
	}
	std::optional<uint32_t> subpass{};
	std::optional<float> line_width{};
	std::optional<float> depth_bias{};
	std::optional<float> depth_slope{};
	std::optional<float> blend{};
	std::optional<float> alpha_blend{};
	std::optional<uint32_t> color_write_mask{};

	updateParameter(param_source, parameters.cull_mode, "cull",
		{
			{ "none", VK_CULL_MODE_NONE },
			{ "front", VK_CULL_MODE_FRONT_BIT },
			{ "back", VK_CULL_MODE_BACK_BIT },
			{ "front and back", VK_CULL_MODE_FRONT_AND_BACK }
		});

	updateParameter(param_source, parameters.polygon_mode, "polygon",
		{
			{ "fill", VK_POLYGON_MODE_FILL },
			{ "line", VK_POLYGON_MODE_LINE },
			{ "point", VK_POLYGON_MODE_POINT }
		});

	updateParameter(param_source, parameters.front_face, "front_face",
		{
			{ "clockwise", VK_FRONT_FACE_CLOCKWISE },
			{ "counter clockwise", VK_FRONT_FACE_COUNTER_CLOCKWISE }
		});

	std::vector<std::pair<const char*, std::optional<VkBlendOp>>> blend_ops = 
	{
		{ "+", VK_BLEND_OP_ADD },
		{ "-", VK_BLEND_OP_SUBTRACT },
		{ "reverse -", VK_BLEND_OP_REVERSE_SUBTRACT },
		{ "min", VK_BLEND_OP_MIN },
		{ "max", VK_BLEND_OP_MAX }
	};

	updateParameter(param_source, parameters.blend_op, "blend_op", blend_ops);
	if (parameters.blend_op.has_value()) 
		parameters.enabled[(size_t)EnableTag::BLEND] = true;

	updateParameter(param_source, parameters.alpha_blend_op, "alpha_blend_op", blend_ops);
	if (parameters.alpha_blend_op.has_value()) 
		parameters.enabled[(size_t)EnableTag::BLEND] = true;

	updateParameter(param_source, parameters.depth_compare_op, "depth_compare_op", 
		{ 
			{ "always", VK_COMPARE_OP_ALWAYS },
			{ "never", VK_COMPARE_OP_NEVER },
			{ "==", VK_COMPARE_OP_EQUAL },
			{ "!=", VK_COMPARE_OP_NOT_EQUAL },
			{ ">", VK_COMPARE_OP_GREATER },
			{ "<", VK_COMPARE_OP_LESS },
			{ ">=", VK_COMPARE_OP_GREATER_OR_EQUAL },
			{ "<=", VK_COMPARE_OP_LESS_OR_EQUAL }
		});

	//TODO: Maybe parse vertex layout

	return shader_sources;
}

shaderc::CompileOptions Shader::getCompileOptions()
{
	shaderc::CompileOptions options{};
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shadercApiVersion(Graphics::API_VERSION));
	options.SetForcedVersionProfile(450, shaderc_profile_core);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	options.AddMacroDefinition("MAX_IMAGE_SLOTS", std::to_string(Graphics::MAX_IMAGE_SLOTS));
	options.AddMacroDefinition("MAX_LIGHTS", std::to_string(Renderer::MAX_LIGHTS));
	options.AddMacroDefinition("DIFFUSE_TEXTURE", "0");
	options.AddMacroDefinition("NORMAL_TEXTURE", "1");
	options.AddMacroDefinition("AO_TEXTURE", "2");
	options.AddMacroDefinition("HEIGHT_TEXTURE", "3");
	options.AddMacroDefinition("SPECULAR_TEXTURE", "4");
	options.AddMacroDefinition("EMMISIVE_TEXTURE", "5");
	options.SetIncluder(std::make_unique<Includer>());
	options.SetGenerateDebugInfo();
	return options;
}

template<typename T>
void Shader::updateParameter(const std::string& source, T& parameter_value, const std::function<T(std::string_view)>& update_function, const char* parameter_name)
{
	std::string token = std::string("#") + parameter_name;
	size_t pos = source.find(token, 0);
	if (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + token.size() + 1;
		std::string value = source.substr(begin, eol - begin);
		parameter_value = update_function(value);
	}
}

template<typename T>
void Shader::updateParameter(const std::string& source, T& parameter_value, const char* parameter_name, const std::vector<std::pair<const char*, T>>& value_pairs)
{
	updateParameter<T>(source, parameter_value, [&](std::string_view value)
		{
			for (const auto& [str, val] : value_pairs)
				if (value == str)
					return val;
			SK_ERROR("Invalid value({}) for parameter: {}", value.data(), parameter_name);
			return T{};
		}, parameter_name);
}

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage, VkDescriptorType type)
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

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage)
{
	auto ranges = compiler.get_active_buffer_ranges(resources.push_constant_buffers.front().id);
	for (auto& range : ranges)
	{
		for (auto&& [name, push_constant] : push_constants)
		{
			if (push_constant.size == range.range && push_constant.offset == range.offset)
			{
				push_constant.stageFlags |= stage;
				return;
			}
		}
		VkPushConstantRange push_constant_range{};
		push_constant_range.offset = range.offset;
		push_constant_range.size = range.range;
		push_constant_range.stageFlags = stage;
		push_constants.emplace(spirv_resource.name, std::move(push_constant_range));
	}
}

Shader::Type Shader::getStringType(std::string_view shader_string)
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
	case Type::NONE:
	case Type::VERTEX: return shaderc_vertex_shader;
	case Type::FRAGMENT: return shaderc_fragment_shader;
	case Type::GEOMETRY: return shaderc_geometry_shader;
	case Type::COMPUTE: return shaderc_compute_shader;
	case Type::TESSELATION_CONTROL: return shaderc_tess_control_shader;
	case Type::TESSELATION_EVALUATION: return shaderc_tess_evaluation_shader;
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

VkShaderStageFlagBits Shader::toVulkanType(Type shader_type)
{
	switch (shader_type)
	{
	case Type::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
	case Type::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case Type::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case Type::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
	case Type::TESSELATION_CONTROL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case Type::TESSELATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	SK_ERROR("Unsupported shader type specified: {0}.", shader_type);
	return VkShaderStageFlagBits(0);
}

Shader::Type Shader::fromVulkanType(VkShaderStageFlagBits vulkan_type)
{
	switch (vulkan_type)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: return Type::VERTEX;
	case VK_SHADER_STAGE_FRAGMENT_BIT: return Type::FRAGMENT;
	case VK_SHADER_STAGE_GEOMETRY_BIT: return Type::GEOMETRY;
	case VK_SHADER_STAGE_COMPUTE_BIT: return Type::COMPUTE;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return Type::TESSELATION_CONTROL;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return Type::TESSELATION_EVALUATION;
	}

	SK_ERROR("Unsupported vulkan shader type specified: {0}.", vulkan_type);
	return Type(0);
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