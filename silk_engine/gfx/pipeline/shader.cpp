#include "shader.h"
#include "io/file.h"
#include "gfx/devices/logical_device.h"
#include "gfx/renderer.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

shaderc::Compiler Shader::compiler{};

Shader::Stage::Stage(const path& file)
	: file(file)
{
	using enum Type;

	path file_extension = file.extension();
	if		(file_extension == ".vert") type = VERTEX;
	else if (file_extension == ".frag") type = FRAGMENT;
	else if (file_extension == ".geom") type = GEOMETRY;
	else if (file_extension == ".comp") type = COMPUTE;
	else if (file_extension == ".tesc") type = TESSELATION_CONTROL;
	else if (file_extension == ".tese") type = TESSELATION_EVALUATION;
}

Shader::Stage::~Stage()
{
	Graphics::logical_device->destroyShaderModule(module);
}

bool Shader::Stage::compile()
{
	if (module)
	{
		Graphics::logical_device->destroyShaderModule(module);
		module = nullptr;
	}
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
	options.SetForcedVersionProfile(450, shaderc_profile_none);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	options.SetSourceLanguage(shaderc_source_language_glsl);
	options.SetTargetSpirv(shaderc_spirv_version_1_6);
	options.SetWarningsAsErrors();
	options.SetGenerateDebugInfo();

	std::string source;

	std::string defines;
	defines += "#define MAX_IMAGE_SLOTS " + std::to_string(Renderer::MAX_IMAGE_SLOTS) + '\n';
	defines += "#define MAX_LIGHTS " + std::to_string(Renderer::MAX_LIGHTS) + '\n';
	defines += "#define DIFFUSE_TEXTURE 0\n";
	defines += "#define NORMAL_TEXTURE 1\n";
	defines += "#define AO_TEXTURE 2\n";
	defines += "#define HEIGHT_TEXTURE 3\n";
	defines += "#define SPECULAR_TEXTURE 4\n";
	defines += "#define EMMISIVE_TEXTURE 5\n";
	source += defines;

	source += File::read(file, std::ios::binary);

	shaderc_shader_kind shaderc_type = (shaderc_shader_kind)0;
	using enum Type;
	switch (type)
	{
	case VERTEX: shaderc_type = shaderc_vertex_shader; break;
	case FRAGMENT: shaderc_type = shaderc_fragment_shader; break;
	case GEOMETRY: shaderc_type = shaderc_geometry_shader; break;
	case COMPUTE: shaderc_type = shaderc_compute_shader; break;
	case TESSELATION_CONTROL: shaderc_type = shaderc_tess_control_shader; break;
	case TESSELATION_EVALUATION: shaderc_type = shaderc_tess_evaluation_shader; break;
	}
	auto compilation_result = compiler.CompileGlslToSpv(source.data(), shaderc_type, file.string().c_str(), options);
	if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		SK_ERROR("Shader stage({}) failed: {}", file, compilation_result.GetErrorMessage());
		return false;
	}

	if (compilation_result.GetNumWarnings())
		SK_WARN("Shader stage({}) warning: {}", file, compilation_result.GetErrorMessage());

	binary = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend()); 

	createModule(); 
	saveCache();

	return true;
}

void Shader::Stage::createModule()
{
	SK_VERIFY(!module, "Module has already been created");
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

path Shader::Stage::getCachePath() const
{
	return path("res/cache/shaders") / (file.string() + ".spv");
}

Shader::Shader(std::string_view name)
{
	std::vector<path> source_files;
	for (const auto& file : std::filesystem::directory_iterator("res/shaders"))
		if (file.path().stem() == name)
			source_files.push_back(file);
	stages.reserve(source_files.size());
	for (const auto& file : source_files)
		stages.emplace_back(makeUnique<Stage>(file));
	std::sort(stages.begin(), stages.end(), [](const unique<Stage>& l, const unique<Stage>& r)  { return l->type < r->type; });
	compile();
}

void Shader::compile()
{
	bool compiled = true;
	for (auto& stage : stages)
		if (!stage->compile())
		{
			compiled = false;
			break;
		}
	if (compiled)
		reflect();
}

void Shader::reflect()
{
	using namespace spirv_cross;
	reflection_data = {};
	for (const auto& stage : stages)
	{
		auto type = stage->type;
		spirv_cross::Compiler compiler(stage->binary);
		spirv_cross::ShaderResources shader_resources;
		try { shader_resources = compiler.get_shader_resources(); }
		catch (const spirv_cross::CompilerError& e) { SK_ERROR(e.what()); }
	
		for (const spirv_cross::Resource& sampled_image : shader_resources.sampled_images)
			loadResource(sampled_image, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	
		for (const spirv_cross::Resource& seperate_image : shader_resources.separate_images)
			loadResource(seperate_image, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

		for (const spirv_cross::Resource& seperate_sampler : shader_resources.separate_samplers)
			loadResource(seperate_sampler, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_SAMPLER);
	
		for (const spirv_cross::Resource& storage_buffer : shader_resources.storage_buffers)
			loadResource(storage_buffer, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	
		for (const spirv_cross::Resource& storage_image : shader_resources.storage_images)
			loadResource(storage_image, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	
		for (const spirv_cross::Resource& subpass_input : shader_resources.subpass_inputs)
			loadResource(subpass_input, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
	
		for (const spirv_cross::Resource& uniform_buffer : shader_resources.uniform_buffers)
			loadResource(uniform_buffer, compiler, shader_resources, type, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	
		for (const spirv_cross::Resource& push_constant : shader_resources.push_constant_buffers)
			loadPushConstant(push_constant, compiler, shader_resources, type);
	
		if (type == Stage::Type::VERTEX)
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
	
				std::string_view token = "instance_";
				if (input.name.size() >= token.size())
					element.instanced = std::string_view(input.name.data(), token.size()) == token;
				else 
					element.instanced = false;
	
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
			shader_constant = { constant.constant_id, shader_constant.stage | (VkShaderStageFlags)type };
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
		reflection_data.descriptor_sets.at(resource.set)->add(resource.binding, resource.count, resource.type, resource.stage);
		reflection_data.resource_locations.emplace(resource.name, ResourceLocation{ resource.set, resource.binding });
	}

	reflection_data.descriptor_set_layouts.reserve(reflection_data.descriptor_sets.size());
	for (auto&& [set, descriptor_set] : reflection_data.descriptor_sets)
	{
		descriptor_set->build();
		reflection_data.descriptor_set_layouts.emplace_back(descriptor_set->getLayout());
	}
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

void Shader::pushConstants(std::string_view name, const void* data) const
{
	const auto& push_constant = reflection_data.push_constant_map.at(name);
	Graphics::submit([&](CommandBuffer& cb) { cb.pushConstants(push_constant.stageFlags, push_constant.offset, push_constant.size, data); });
}

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage, VkDescriptorType type)
{
	for (Resource& resource : reflection_data.resources)
	{
		if (resource.name == spirv_resource.name)
		{
			resource.stage |= (VkShaderStageFlags)stage;
			return;
		}
	}

	const spirv_cross::SPIRType& spir_type = compiler.get_type(spirv_resource.type_id);
	uint32_t set = compiler.get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
	uint32_t binding = compiler.get_decoration(spirv_resource.id, spv::DecorationBinding);

	reflection_data.descriptor_sets.try_emplace(set, makeShared<DescriptorSet>());

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
	resource.stage = (VkShaderStageFlags)stage;
	resource.type = type;
	resource.name = spirv_resource.name;
	reflection_data.resources.emplace_back(std::move(resource));
}

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage)
{
	auto ranges = compiler.get_active_buffer_ranges(resources.push_constant_buffers.front().id);
	for (auto& range : ranges)
	{
		for (auto& push_constant : reflection_data.push_constants)
		{
			if (push_constant.size == range.range && push_constant.offset == range.offset)
			{
				push_constant.stageFlags |= (VkShaderStageFlags)stage;
				return;
			}
		}
		VkPushConstantRange push_constant_range{};
		push_constant_range.offset = range.offset;
		push_constant_range.size = range.range;
		push_constant_range.stageFlags = (VkShaderStageFlags)stage;
		reflection_data.push_constants.emplace_back(push_constant_range);
		reflection_data.push_constant_map.emplace(spirv_resource.name, push_constant_range);
	}
}