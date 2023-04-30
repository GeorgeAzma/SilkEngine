#include "shader.h"
#include "io/file.h"
#include "gfx/devices/logical_device.h"
#include "gfx/debug_renderer.h"
#include "gfx/instance.h"
#define STB_INCLUDE_IMPLEMENTATION
#include "stb_include.h"
#include "gfx/render_context.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include <spirv_cross/spirv_cross.hpp>
#include <shaderc/shaderc.hpp>

Shader::Stage::Stage(const path& file)
	: file(file)
{
	using enum Type;

	path file_extension = file.extension();
	if (file_extension == ".vert") type = VERTEX;
	else if (file_extension == ".frag") type = FRAGMENT;
	else if (file_extension == ".geom") type = GEOMETRY;
	else if (file_extension == ".comp") type = COMPUTE;
	else if (file_extension == ".tesc") type = TESSELATION_CONTROL;
	else if (file_extension == ".tese") type = TESSELATION_EVALUATION;
}

Shader::Stage::~Stage()
{
	saveCache();
	RenderContext::getLogicalDevice().destroyShaderModule(module);
}

bool Shader::Stage::compile()
{
	DebugTimer t(std::format("Compiling {}", file.filename()));
	static shaderc::Compiler compiler;

	if (module)
	{
		RenderContext::getLogicalDevice().destroyShaderModule(module);
		module = nullptr;
	}

	shaderc_env_version api_version = shaderc_env_version_vulkan_1_3;
	switch (RenderContext::getInstance().getVulkanVersion())
	{
	case VulkanVersion::VULKAN_1_0: api_version = shaderc_env_version_vulkan_1_0; break;
	case VulkanVersion::VULKAN_1_1: api_version = shaderc_env_version_vulkan_1_1; break;
	case VulkanVersion::VULKAN_1_2: api_version = shaderc_env_version_vulkan_1_2; break;
	case VulkanVersion::VULKAN_1_3: api_version = shaderc_env_version_vulkan_1_3; break;
	}

	shaderc::CompileOptions options{};
	options.SetTargetEnvironment(shaderc_target_env_vulkan, api_version);
	options.SetForcedVersionProfile(450, shaderc_profile_none);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	options.SetSourceLanguage(shaderc_source_language_glsl);
	options.SetWarningsAsErrors();
	options.SetGenerateDebugInfo();

	// TODO: remove this, make this a parameter, or do something else
	std::string source; // I am not using shaderc's pre processing because it's 30% slower
	source += "#define MAX_IMAGE_SLOTS " + std::to_string(DebugRenderer::MAX_IMAGE_SLOTS) + '\n';
	source += "#define MAX_LIGHTS " + std::to_string(DebugRenderer::MAX_LIGHTS) + '\n';
	source += "#define DIFFUSE_TEXTURE 0\n";
	source += "#define NORMAL_TEXTURE 1\n";
	source += "#define AO_TEXTURE 2\n";
	source += "#define HEIGHT_TEXTURE 3\n";
	source += "#define SPECULAR_TEXTURE 4\n";
	source += "#define EMMISIVE_TEXTURE 5\n";
	source += stb_include_file((char*)file.string().c_str(), nullptr, nullptr, nullptr);

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
		SK_ERROR("Error in {}: {}", file, compilation_result.GetErrorMessage());
		return false;
	}
	if (compilation_result.GetNumWarnings())
		SK_WARN("Warning in {}: {}", file, compilation_result.GetErrorMessage());

	binary = std::vector<uint32_t>(compilation_result.cbegin(), compilation_result.cend());

	createModule();

	return true;
}

void Shader::Stage::createModule()
{
	SK_VERIFY(!module, "Module has already been created");
	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = binary.size() * sizeof(uint32_t);
	ci.pCode = binary.data();
	module = RenderContext::getLogicalDevice().createShaderModule(ci);
}

void Shader::Stage::loadCache()
{
	File::read(getCachePath(), binary, std::ios::binary);
}

void Shader::Stage::saveCache() const
{
	if (!std::filesystem::exists("res/cache/shaders"))
		std::filesystem::create_directories("res/cache/shaders");
	File::write(getCachePath(), binary.data(), binary.size() * sizeof(uint32_t));
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
	SK_VERIFY(source_files.size(), "Shader {} does not exist", name);
	stages.reserve(source_files.size());
	for (const auto& file : source_files)
		stages.emplace_back(makeUnique<Stage>(file));
	std::ranges::sort(stages);
	compile();
}

Shader::Shader(const std::vector<std::string_view>& names)
{
	for (const auto& file : std::filesystem::directory_iterator("res/shaders"))
		for (const auto& name : names)
			if (file.path().stem() == name)
				stages.emplace_back(makeUnique<Stage>(file));
	SK_ASSERT(stages.size() == names.size(), "Couldn't find one of the shader stage files, make sure paths are correct");
	std::ranges::sort(stages);
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

		for (const spirv_cross::Resource& push_constant_buffer : shader_resources.push_constant_buffers)
		{
			auto active_ranges = compiler.get_active_buffer_ranges(push_constant_buffer.id);
			for (auto& active_range : active_ranges)
			{
				bool found_push_constant = false;
				for (size_t i = 0; i < reflection_data.push_constants.size(); ++i)
				{
					auto& existing_push_constant = reflection_data.push_constants[i];
					if (existing_push_constant.offset + existing_push_constant.size >= active_range.offset && existing_push_constant.offset < active_range.offset + active_range.range)
					{
						existing_push_constant.stageFlags |= (VkShaderStageFlags)type;
						found_push_constant = true;
						break;
					}
				}
				if (!found_push_constant)
				{
					VkPushConstantRange push_constant_range{};
					push_constant_range.offset = active_range.offset;
					push_constant_range.size = active_range.range;
					push_constant_range.stageFlags = (VkShaderStageFlags)type;
					reflection_data.push_constants.emplace_back(push_constant_range);
				}
			}
		}

		if (type & Stage::VERTEX)
		{
			struct VertexAttribute
			{
				uint32_t location = 0;
				uint32_t size = 0;
				VkFormat format = VkFormat(0);
				bool instanced = false;
				bool operator<(const VertexAttribute& other) const { return location < other.location; }
			};
			//TODO: Support things like array/struct input vertex attributes
			std::vector<VertexAttribute> vertex_attributes;
			for (auto& input : shader_resources.stage_inputs)
			{

				uint32_t location = compiler.get_decoration(input.id, spv::Decoration::DecorationLocation);
				const auto& type = compiler.get_type(input.base_type_id);
				uint32_t size = 0;
				VkFormat format = VkFormat(0);
				switch (type.basetype)
				{
					using Type = spirv_cross::SPIRType::BaseType;
				case Type::Float:
					size = 4;
					format = makeArray(VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT)[type.vecsize - 1];
					break;
				case Type::Int:
					size = 4;
					format = makeArray(VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT)[type.vecsize - 1];
					break;
				case Type::UInt:
					size = 4;
					format = makeArray(VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT)[type.vecsize - 1];
					break;
				case Type::Double:
					size = 8;
					format = makeArray(VK_FORMAT_R64_SFLOAT, VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT)[type.vecsize - 1];
					break;
				case Type::UByte:
					size = 1;
					format = makeArray(VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8A8_UINT)[type.vecsize - 1];
					break;
				case Type::SByte:
					size = 1;
					format = makeArray(VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8A8_SINT)[type.vecsize - 1];
					break;
				case Type::UShort:
					size = 2;
					format = makeArray(VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16A16_UINT)[type.vecsize - 1];
					break;
				case Type::Short:
					size = 2;
					format = makeArray(VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16A16_SINT)[type.vecsize - 1];
					break;
				}
				size *= type.vecsize;
				size *= type.columns;

				// TODO: Not the greatest idea, but works in most cases
				std::string_view token = "instance_";
				bool is_input_instanced = (input.name.size() >= token.size() && std::string_view(input.name.data(), token.size()) == token);
				vertex_attributes.emplace_back(location, size, format, is_input_instanced);
			}
			if (vertex_attributes.size())
			{
				std::ranges::sort(vertex_attributes, std::less<VertexAttribute>{});

				uint32_t offset = 0;
				uint32_t instance_offset = 0;

				bool is_instanced = false;

				for (const VertexAttribute& vertex_attribute : vertex_attributes)
				{
					uint32_t actual_rows = (float)vertex_attribute.size / sizeof(vec4);
					uint32_t remainder = vertex_attribute.size % sizeof(vec4);
					uint32_t rows = actual_rows + (remainder > 0);
					for (uint32_t i = 0; i < rows; ++i)
					{
						VkVertexInputAttributeDescription attribute_desc{};
						attribute_desc.format = vertex_attribute.format;
						attribute_desc.location = vertex_attribute.location + i;

						if (vertex_attribute.instanced)
						{
							is_instanced = true;
							attribute_desc.offset = instance_offset;
							attribute_desc.binding = 1;
							instance_offset += i < actual_rows ? sizeof(vec4) : remainder;
						}
						else
						{
							attribute_desc.offset = offset;
							attribute_desc.binding = 0;
							offset += i < actual_rows ? sizeof(vec4) : remainder;
						}

						reflection_data.vertex_input_attribute_descriptions.emplace_back(std::move(attribute_desc));
					}
				}

				reflection_data.vertex_input_binding_descriptions.resize(1 + is_instanced);
				auto& vertex_binding_desc = reflection_data.vertex_input_binding_descriptions[0];
				vertex_binding_desc.binding = 0;
				vertex_binding_desc.stride = offset;
				vertex_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				if (is_instanced)
				{
					auto& instance_binding_desc = reflection_data.vertex_input_binding_descriptions[1];
					instance_binding_desc.binding = 1;
					instance_binding_desc.stride = instance_offset;
					instance_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				}
			}
		}

		if (type & Stage::FRAGMENT)
			for (const spirv_cross::Resource& output : shader_resources.stage_outputs)
				reflection_data.render_targets[compiler.get_decoration(output.id, spv::DecorationLocation)] = output.name;

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

	std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptor_set_layout_bindings;
	for (const auto& resource : reflection_data.resources)
	{
		VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
		descriptor_set_layout_binding.binding = resource.binding;
		descriptor_set_layout_binding.descriptorCount = resource.count;
		descriptor_set_layout_binding.descriptorType = resource.type;
		descriptor_set_layout_binding.pImmutableSamplers = nullptr;
		descriptor_set_layout_binding.stageFlags = (VkShaderStageFlags) resource.stage;
		descriptor_set_layout_bindings[resource.set].emplace_back(std::move(descriptor_set_layout_binding));
		reflection_data.resource_locations.emplace(resource.name, ResourceLocation{ resource.set, resource.binding });
	}

	for (auto&& [set, bindings] : descriptor_set_layout_bindings)
		reflection_data.descriptor_set_layouts.emplace(set, DescriptorSetLayout::get(bindings));
}

Shader::ResourceLocation Shader::getLocation(std::string_view resource_name) const
{
	if (auto resource_location = reflection_data.resource_locations.find(resource_name); resource_location != reflection_data.resource_locations.end())
		return resource_location->second;
	return ResourceLocation{};
}

void Shader::pushConstant(const void* data, Stage::Type stages, uint32_t size, uint32_t offset) const
{
	RenderContext::record([&](CommandBuffer& cb) { cb.pushConstants((VkPipelineStageFlags)stages, offset, size, data); });
}

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, Stage::Type stage, VkDescriptorType type)
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