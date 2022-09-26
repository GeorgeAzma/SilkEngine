#include "shader.h"
#include "io/file.h"
#include "gfx/devices/logical_device.h"
#include "gfx/renderer.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

//void __CRTDECL operator delete(void* _Block, size_t _Size) noexcept {}

bool Shader::Stage::compile(std::string_view source, const shaderc::Compiler& compiler, const shaderc::CompileOptions& options, bool skip_if_error)
{
	shaderc_shader_kind shaderc_type = (shaderc_shader_kind)0;
	switch (type)
	{
	case Type::VERTEX: shaderc_type = shaderc_vertex_shader; break;
	case Type::FRAGMENT: shaderc_type = shaderc_fragment_shader; break;
	case Type::GEOMETRY: shaderc_type = shaderc_geometry_shader; break;
	case Type::COMPUTE: shaderc_type = shaderc_compute_shader; break;
	case Type::TESSELATION_CONTROL: shaderc_type = shaderc_tess_control_shader; break;
	case Type::TESSELATION_EVALUATION: shaderc_type = shaderc_tess_evaluation_shader; break;
	}
	shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(source.data(), shaderc_type, file.data(), options);
	if (skip_if_error && compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		SK_WARN("{} Shader was modified and has errors, using the older version", file.data());
		return false;
	}
	SK_ASSERT(compilation_result.GetCompilationStatus() == shaderc_compilation_status_success, compilation_result.GetErrorMessage());
	binary = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend());
	return true;
}

void Shader::Stage::createModule()
{
	SK_ASSERT(!module, "Module has already been created");
	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = binary.size() * sizeof(uint32_t);
	ci.pCode = binary.data();
	module = Graphics::logical_device->createShaderModule(ci);
}

void Shader::Stage::loadCache()
{
	std::ifstream cache(getCachePath(), std::ios::binary | std::ios::ate);
	size_t size = cache.tellg();
	binary.resize(size / sizeof(uint32_t));
	cache.seekg(std::ios::beg);
	cache.read((char*)binary.data(), size);
}

void Shader::Stage::saveCache() const
{
	std::ofstream cache(getCachePath(), std::ios::binary);
	cache.write((const char*)binary.data(), binary.size() * sizeof(uint32_t));
}

void Shader::Stage::destroy()
{
	if (module)
		Graphics::logical_device->destroyShaderModule(module);
}

Shader::Shader(std::string_view file, const std::vector<Define>& defines)
	: file(file), defines(defines), last_compiled(Time::file_start)
{
	this->defines.emplace_back("MAX_IMAGE_SLOTS", std::to_string(Renderer::MAX_IMAGE_SLOTS));
	this->defines.emplace_back("MAX_LIGHTS", std::to_string(Renderer::MAX_LIGHTS));
	this->defines.emplace_back("DIFFUSE_TEXTURE", "0");
	this->defines.emplace_back("NORMAL_TEXTURE", "1");
	this->defines.emplace_back("AO_TEXTURE", "2");
	this->defines.emplace_back("HEIGHT_TEXTURE", "3");
	this->defines.emplace_back("SPECULAR_TEXTURE", "4");
	this->defines.emplace_back("EMMISIVE_TEXTURE", "5");
	compile();
}

Shader::~Shader()
{
	for (auto& stage : stages)
		stage.destroy();
}

void Shader::compile()
{
	std::string path = std::string("res/shaders") + '/' + file + ".glsl";
	double last_modified = std::chrono::duration_cast<std::chrono::duration<double>>(std::filesystem::last_write_time(path).time_since_epoch()).count();
	bool file_changed = last_modified >= last_compiled;
	last_compiled = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::file_clock::now().time_since_epoch()).count();
	
	auto get_compile_options = []
	{
		shaderc_env_version api_version;
		switch (Graphics::API_VERSION)
		{
		case APIVersion::VULKAN_1_0: api_version = shaderc_env_version_vulkan_1_0; break;
		case APIVersion::VULKAN_1_1: api_version = shaderc_env_version_vulkan_1_1; break;
		case APIVersion::VULKAN_1_2: api_version = shaderc_env_version_vulkan_1_2; break;
		case APIVersion::VULKAN_1_3: api_version = shaderc_env_version_vulkan_1_3; break;
		}
		shaderc::CompileOptions options{};
		options.SetTargetEnvironment(shaderc_target_env_vulkan, api_version);
		options.SetForcedVersionProfile(450, shaderc_profile_core);
		//shaderc_optimization_level_performance crashes for some reason, if you enable it and don't wanna crash, uncomment code above (operator delete() code), note memory will leak while the code is uncommented, so only uncomment when you are compiling shaders first time
		//options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetGenerateDebugInfo();
		return options;
	};

	if (file_changed)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options = get_compile_options();

		Parameters params{};
		std::unordered_map<uint32_t, std::string> sources = parse(path, params);
		std::vector<Shader::Stage> new_stages;
		new_stages.resize(sources.size());

		size_t index = 0;
		bool all_succeeded = true;
		for (auto&& [type, source] : sources)
		{
			new_stages[index].type = (Stage::Type)type;
			new_stages[index].file = file;
			if (!new_stages[index].compile(source, compiler, options, true))
			{
				all_succeeded = false;
				break;
			}
			++index;
		}
		if (all_succeeded)
		{
			for (auto& stage : stages)
				stage.destroy();
			stages = std::move(new_stages);
			parameters = params;
			for (auto& stage : stages)
			{
				stage.createModule();
				stage.saveCache();
			}
			reflect();
		}
		else
		{

		}
		return;
	}
	
	bool not_compiled = stages.empty();
	if (not_compiled)
	{
		std::unordered_map<uint32_t, std::string> sources = parse(path, parameters);
		stages.resize(sources.size());
		shaderc::Compiler compiler;
		shaderc::CompileOptions options = get_compile_options();
		size_t index = 0;
		for (auto&& [type, source] : sources)
		{;
			stages[index].type = (Stage::Type)type;
			stages[index].file = file;
			std::ifstream cache(stages[index].getCachePath(), std::ios::binary);
			if (cache)
			{
				stages[index].loadCache();
				stages[index].createModule();
			}
			else
			{
				stages[index].compile(source, compiler, options);
				stages[index].createModule();
				stages[index].saveCache();
			}
			++index;
		}
		reflect();
		return;
	}
}

void Shader::reflect()
{
	reflection_data = {};
	for (const auto& stage : stages)
	{
		spirv_cross::Compiler compiler(stage.binary);
		spirv_cross::ShaderResources shader_resources = compiler.get_shader_resources();
		auto vulkan_stage = Stage::toVulkanType(stage.type);

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

		if (stage.type == Stage::Type::VERTEX)
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

			reflection_data.buffer_layout = BufferLayout(aligned_buffer_elements);
		}

		auto constants = compiler.get_specialization_constants();
		for (auto& constant : constants)
		{
			auto& shader_constant = reflection_data.constants[compiler.get_name(constant.id)];
			shader_constant = { constant.constant_id, shader_constant.stage | vulkan_stage };
		}

		//Local size reflection
		reflection_data.local_size = uvec3(0);
		reflection_data.local_size[0] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
		reflection_data.local_size[1] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
		reflection_data.local_size[2] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
		if (reflection_data.local_size != uvec3(0))
			reflection_data.local_size = math::max(reflection_data.local_size, uvec3(1));
	}

	for (const auto& resource : reflection_data.resources)
	{
		auto& set = reflection_data.descriptor_sets.at(resource.set);
		set->add(resource.binding, resource.count, resource.type, resource.stage);
		reflection_data.resource_locations.emplace(resource.name, ResourceLocation{ resource.set, resource.binding });
	}

	for (auto& descriptor_set : reflection_data.descriptor_sets)
		descriptor_set.second->build();
}

void Shader::set(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos)
{
	const auto& resource_location = reflection_data.resource_locations.at(resource_name);
	reflection_data.descriptor_sets.at(resource_location.set)->setBufferInfo(resource_location.binding, buffer_infos);
}

void Shader::set(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos)
{
	const auto& resource_location = reflection_data.resource_locations.at(resource_name);
	reflection_data.descriptor_sets.at(resource_location.set)->setImageInfo(resource_location.binding, image_infos);
}

void Shader::setIfExists(std::string_view resource_name, const std::vector<VkDescriptorBufferInfo>& buffer_infos)
{
	auto resource_location = reflection_data.resource_locations.find(resource_name);
	if(resource_location != reflection_data.resource_locations.end())
		reflection_data.descriptor_sets.at(resource_location->second.set)->setBufferInfo(resource_location->second.binding, buffer_infos);
}

void Shader::setIfExists(std::string_view resource_name, const std::vector<VkDescriptorImageInfo>& image_infos)
{
	auto resource_location = reflection_data.resource_locations.find(resource_name);
	if (resource_location != reflection_data.resource_locations.end())
		reflection_data.descriptor_sets.at(resource_location->second.set)->setImageInfo(resource_location->second.binding, image_infos);
}

const Shader::ResourceLocation* Shader::getIfExists(std::string_view resource_name) const
{
	auto resource_location = reflection_data.resource_locations.find(resource_name);
	return (resource_location != reflection_data.resource_locations.end()) ? &resource_location->second : nullptr;
}

void Shader::bindDescriptorSets()
{
	for (auto&& [set, descriptor_set] : reflection_data.descriptor_sets)
		descriptor_set->bind(set);
}

std::unordered_map<uint32_t, std::string> Shader::parse(std::string_view file, Parameters& parameters)
{
	std::unordered_map<uint32_t, std::string> shader_sources;

	std::vector<uint8_t> src = File::read(file);
	std::string_view source((const char*)src.data(), src.size());

	std::string defines_str;
	for (const auto& define : defines)
		defines_str += std::string("#define ") + define.name + ' ' + define.value + '\n';


	//Devide shader codes
	size_t off;
	Stage::Type type;
	std::string_view value = getPreprocessorValue(source, "type");
	std::string_view param_source = std::string_view(source).substr(0, size_t(value.data() - source.data() - 6));
	while (value.size())
	{
		off = size_t(value.data() - source.data() + value.size());
		type = Stage::getStringType(value);
		value = getPreprocessorValue(source, "type", off);
		size_t source_size = (value.data() ? size_t(value.data() - source.data() - 6) : source.size()) - off;
		std::string& shader_source = shader_sources.emplace((uint32_t)type, source.substr(off, source_size)).first->second;
		
		shader_source.insert(0, defines_str);

		std::string_view include = getPreprocessorValue(shader_source, "include");
		while (include.size())
		{
			std::string include_str;
			include_str.append(include.data(), include.size());
			std::string path = std::string("res/shaders/") + include_str.data() + ".glsl";
			std::vector<uint8_t> include_src = File::read(path);
			std::string_view include_source((const char*)include_src.data(), include_src.size());
			size_t include_pos = include.data() - shader_source.data() - 9;
			shader_source.insert(include_pos, include_source);
			shader_source.erase(include_pos + include_source.size(), 9 + include.size());
			include = getPreprocessorValue(shader_source, "include", include_pos);
		}
	}

	if (param_source.empty()) 
		return shader_sources;

	//Update parameters
	constexpr auto tokens = makeArray("depth_test", "depth_write", "stencil_test", "blend", "sample_shading", "primitive_restart", "rasterizer_discard", "depth_clamp", "depth_bias_enabled", "color_blend_logic_op");
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

	if (auto value = getPreprocessorValue(param_source, "depth_bias"); value.size())
		parameters.depth_bias = std::stof(value.data());
	if (auto value = getPreprocessorValue(param_source, "depth_slope"); value.size())
		parameters.depth_slope = std::stof(value.data());
	if (auto value = getPreprocessorValue(param_source, "line_width"); value.size())
		parameters.line_width = std::stof(value.data());
	if (auto value = getPreprocessorValue(param_source, "subpass"); value.size())
		parameters.subpass = std::stoul(value.data());

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

	std::vector<std::pair<std::string_view, std::optional<VkBlendOp>>> blend_ops = 
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

	return shader_sources;
}

std::string_view Shader::getPreprocessorValue(std::string_view source, std::string_view preprocessor_name, size_t offset)
{
	std::string token = std::string("#") + preprocessor_name.data();
	size_t pos = source.find(token, offset);
	if (pos != std::string::npos)
	{
		size_t begin = pos + token.size() + 1;
		size_t eol = source.find_first_of("\r\n", begin);
		return source.substr(begin, eol - begin);
	}
	return {};
}

template<typename T>
void Shader::updateParameter(std::string_view source, T& parameter_value, const std::function<T(std::string_view)>& update_function, std::string_view parameter_name)
{
	std::string token = std::string("#") + parameter_name.data();
	size_t pos = source.find(token, 0);
	if (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);
		SK_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + token.size() + 1;
		std::string_view value = source.substr(begin, eol - begin);
		parameter_value = update_function(value);
	}
}

template<typename T>
void Shader::updateParameter(std::string_view source, T& parameter_value, std::string_view parameter_name, const std::vector<std::pair<std::string_view, T>>& value_pairs)
{
	updateParameter<T>(source, parameter_value, [&](std::string_view value)
		{
			for (auto&& [str, val] : value_pairs)
				if (value == str)
					return val;
			return T{};
		}, parameter_name);
}

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage, VkDescriptorType type)
{
	for (Resource& resource : reflection_data.resources)
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

	if (reflection_data.descriptor_sets.find(set) == reflection_data.descriptor_sets.end())
		reflection_data.descriptor_sets.emplace(set, makeShared<DescriptorSet>());

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
	reflection_data.resources.emplace_back(std::move(resource));
}

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlags stage)
{
	auto ranges = compiler.get_active_buffer_ranges(resources.push_constant_buffers.front().id);
	for (auto& range : ranges)
	{
		for (auto&& [name, push_constant] : reflection_data.push_constants)
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
		reflection_data.push_constants.emplace(spirv_resource.name, std::move(push_constant_range));
	}
}

Shader::Stage::Type Shader::Stage::getStringType(std::string_view shader_string)
{
	using enum Stage::Type;
	if (shader_string == "vertex") return VERTEX;
	else if (shader_string == "fragment") return FRAGMENT;
	else if (shader_string == "geometry") return GEOMETRY;
	else if (shader_string == "compute") return COMPUTE;
	else if (shader_string == "tesselation_control") return TESSELATION_CONTROL;
	else if (shader_string == "tesselation_evaluation") return TESSELATION_EVALUATION;

	SK_ERROR("Unsupported shader stage string specified: {}. try writing #type vertex/fragment/geometry/compute/tesselation_control/tesselation_evaluation", shader_string);
	return Shader::Stage::Type(0);
}

VkShaderStageFlagBits Shader::Stage::toVulkanType(Stage::Type shader_type)
{
	using enum Stage::Type;
	switch (shader_type)
	{
	case VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
	case FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
	case TESSELATION_CONTROL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case TESSELATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	return VkShaderStageFlagBits(0);
}

Shader::Stage::Type Shader::Stage::fromVulkanType(VkShaderStageFlagBits vulkan_type)
{
	using enum Stage::Type;
	switch (vulkan_type)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: return VERTEX;
	case VK_SHADER_STAGE_FRAGMENT_BIT: return FRAGMENT;
	case VK_SHADER_STAGE_GEOMETRY_BIT: return GEOMETRY;
	case VK_SHADER_STAGE_COMPUTE_BIT: return COMPUTE;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return TESSELATION_CONTROL;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return TESSELATION_EVALUATION;
	}

	return Type(0);
}

std::string Shader::Stage::getTypeFileExtension(Shader::Stage::Type shader_type)
{
	using enum Stage::Type;
	switch (shader_type)
	{
	case VERTEX: return "vert";
	case FRAGMENT: return "frag";
	case GEOMETRY: return "geom";
	case COMPUTE: return "comp";
	case TESSELATION_CONTROL: return "tesc";
	case TESSELATION_EVALUATION: return "tese";
	}

	return "";
}