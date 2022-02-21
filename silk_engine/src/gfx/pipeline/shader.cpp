#include "shader.h"
#include "io/file.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "scene/resources.h"
#include "utils/string.h"

Shader::Includer::IncludeResult* Shader::Includer::includeLocal(const char* header, const char* includer, size_t inclusion_depth)
{
	std::filesystem::path directory = "data/shaders/";
	std::string content_str = File::read(directory / header);
	char* content = new char[content_str.size()];

	std::memcpy(content, content_str.c_str(), content_str.size());

	return new IncludeResult(header, content, content_str.size(), content);
}

Shader::Includer::IncludeResult* Shader::Includer::includeSystem(const char* header, const char* includer, size_t inclusion_depth)
{
	std::string content_str = File::read(header);
	char* content = new char[content_str.size()];

	std::memcpy(content, content_str.c_str(), content_str.size());

	return new IncludeResult(header, content, content_str.size(), content);
}

void Shader::Includer::releaseInclude(IncludeResult* result)
{
	if (result)
	{
		delete[] static_cast<char*>(result->userData);
		delete result;
	}
}

Shader::Shader(const std::filesystem::path& file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines);
}

void Shader::compile(const std::vector<Define>& defines)
{
	bool resources_was_compiled = resources.size();
	for (auto& pipeline_shader_stage_info : pipeline_shader_stage_infos)
		vkDestroyShaderModule(*Graphics::logical_device, pipeline_shader_stage_info.module, nullptr);
	pipeline_shader_stage_infos.clear();

	std::vector<Define> all_defines = defines;
	all_defines.emplace_back("MAX_IMAGE_SLOTS", std::to_string(Graphics::MAX_IMAGE_SLOTS));
	std::vector<Extension> all_extensions = {};
	all_extensions.emplace_back("GL_GOOGLE_include_directive", "require");
	all_extensions.emplace_back("GL_ARB_separate_shader_objects", "require");
	all_extensions.emplace_back("GL_ARB_shading_language_420pack", "enable");
	std::stringstream preamble;

	for (const auto& define : all_defines)
		preamble << "#define " << define.name << " " << define.value << '\n';
	for (const auto& extension : all_extensions)
		preamble << "#extension " << extension.name << " : " << extension.behavior << '\n';

	std::string preamble_str = preamble.str();

	std::filesystem::path path = (std::filesystem::path("data/shaders") / file).string() + ".glsl";
	std::filesystem::path cache_path = (std::filesystem::path("data/cache/shaders") / file).string() + ".glsl";
	std::unordered_map<uint32_t, std::string> shader_sources = parse(path);

	auto resources = getResources();

	for (auto&& [type, source] : shader_sources)
	{
		EShLanguage language = getEshLanguage((Type)type);
		glslang::TProgram program;
		glslang::TShader shader(language);

		auto messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);
#ifdef SK_ENABLE_DEBUG_OUTPUT
		messages = EShMessages(messages | EShMsgDebugInfo);
#endif
		
		const char* shader_source = source.c_str();
		const int shader_length = int(source.size());
		std::string shader_name = getTypeFileExtension((Type)type);
		const char* shader_name_cstr = shader_name.c_str();
		shader.setStringsWithLengthsAndNames(&shader_source, nullptr, &shader_name_cstr, 1);
		shader.setPreamble(preamble_str.c_str());
		shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan, 120);
		shader.setEnvClient(glslang::EShClientVulkan, getEshClientVersion(Graphics::API_VERSION));
		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
		
		std::filesystem::path file_cache_path = cache_path.string() + getTypeFileExtension((Type)type) + ".spv";
		std::ifstream in(file_cache_path, std::ios::ate | std::ios::binary);
		std::vector<uint32_t> binary;
#ifndef SK_ENABLE_DEBUG_OUTPUT
		if (in.is_open())
		{
			size_t size = in.tellg();
			in.seekg(0);
			binary.resize(size / sizeof(uint32_t));
			in.read((char*)binary.data(), size);

			SK_TRACE("Shader cache loaded: {0}", file_cache_path);
		}
		else
#endif
		{
			Includer includer;
			std::string str;
			
			if (!shader.preprocess(&resources, getEshClientVersion(Graphics::API_VERSION), ENoProfile, false, false, messages, &str, includer))
				SK_ERROR("[Shader compiler]: SPRIV shader preprocess failed: {0}:\n{1}\n{2}", path, shader.getInfoLog(), shader.getInfoDebugLog());

			if (!shader.parse(&resources, getEshClientVersion(Graphics::API_VERSION), true, messages, includer)) 
				SK_ERROR("[Shader compiler]: SPRIV shader parse failed: {0}:\n{1}\n{2}", path, shader.getInfoLog(), shader.getInfoDebugLog());

			program.addShader(&shader);

			if (!program.link(messages) || !program.mapIO())
				SK_ERROR("[Shader compiler]: Couldn't link shader program");

			glslang::SpvOptions spv_options;
			spv_options.disableOptimizer = false;
#ifdef SK_ENABLE_DEBUG_OUTPUT
			spv_options.stripDebugInfo = false;
			spv_options.generateDebugInfo = true;
#else
			spv_options.generateDebugInfo = false;
#endif
			spv_options.optimizeSize = true;

			spv::SpvBuildLogger logger;
			glslang::GlslangToSpv(*program.getIntermediate(language), binary, &logger, &spv_options);

			std::ofstream out(file_cache_path, std::ios::binary);
			SK_ASSERT(out.is_open(), "Couldn't create shader cache file: {0}", file_cache_path);
			out.write((char*)binary.data(), binary.size() * sizeof(uint32_t));

			SK_TRACE("Shader cache created: {0}", file_cache_path);
		}

		VkShaderStageFlagBits stage_flag = getVulkanType((Type)type);

		VkPipelineShaderStageCreateInfo pipeline_shader_stage_info{};
		pipeline_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipeline_shader_stage_info.stage = stage_flag;
		pipeline_shader_stage_info.module = createShaderModule(binary);
		pipeline_shader_stage_info.pName = "main";

		spirv_cross::Compiler compiler(binary.data(), binary.size());
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		
		if (!resources_was_compiled)
		{
			for (const spirv_cross::Resource& sampled_image : resources.sampled_images)
				loadResource(sampled_image, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

			for (const spirv_cross::Resource& seperate_image : resources.separate_images)
				loadResource(seperate_image, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

			for (const spirv_cross::Resource& seperate_sampler : resources.separate_samplers)
				loadResource(seperate_sampler, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_SAMPLER);

			for (const spirv_cross::Resource& storage_buffer : resources.storage_buffers)
				loadResource(storage_buffer, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

			for (const spirv_cross::Resource& storage_image : resources.storage_images)
				loadResource(storage_image, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			for (const spirv_cross::Resource& subpass_input : resources.subpass_inputs)
				loadResource(subpass_input, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

			for (const spirv_cross::Resource& uniform_buffer : resources.uniform_buffers)
				loadResource(uniform_buffer, compiler, resources, stage_flag, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

			for (const spirv_cross::Resource& push_constant : resources.push_constant_buffers)
				loadPushConstant(push_constant, compiler, resources, stage_flag);

			//Local size reflection
			local_size = glm::uvec3(0);
			local_size[0] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
			local_size[1] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
			local_size[2] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
			if (local_size != glm::uvec3(0))
				local_size = glm::max(local_size, glm::uvec3(1));
		}
		pipeline_shader_stage_infos.emplace_back(std::move(pipeline_shader_stage_info));
	}

	if (!resources_was_compiled)
	{
		for (const auto& resource : this->resources)
			descriptor_sets.at(resource.set)->add(resource.binding, resource.count, resource.type, resource.stage_flags);

		for (auto& descriptor_set : descriptor_sets)
			descriptor_set.second->build();
	}

	SK_TRACE("Shader loaded: {0}", path);
}

Shader::~Shader()
{
	for(auto& pipeline_shader_stage_info : pipeline_shader_stage_infos)
		vkDestroyShaderModule(*Graphics::logical_device, pipeline_shader_stage_info.module, nullptr);
}

std::unordered_map<uint32_t, std::string> Shader::parse(const std::filesystem::path& file)
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

		size_t next_line_pos = source.find_first_not_of("\r\n", eol);
		SK_ASSERT(next_line_pos != std::string::npos, "Syntax error");
		pos = source.find(type_token, next_line_pos);

		shader_sources[(uint32_t)getStringType(type_string)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
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

void Shader::loadResource(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlagBits stage_flag, VkDescriptorType type)
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
		.stage_flags = (VkShaderStageFlags)stage_flag,
		.type = type
	};
	this->resources.emplace_back(std::move(resource));
}

void Shader::loadPushConstant(const spirv_cross::Resource& spirv_resource, const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources, VkShaderStageFlagBits stage_flag)
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
		VkPushConstantRange push_constant_range{};
		push_constant_range.offset = range.offset;
		push_constant_range.size = range.range;
		push_constant_range.stageFlags = stage_flag;
		push_constants.emplace_back(std::move(push_constant_range));
	}
}

EShLanguage Shader::getEshLanguage(Shader::Type shader_type)
{
	switch (shader_type)
	{
	case Shader::Type::VERTEX: return EShLangVertex;
	case Shader::Type::FRAGMENT: return EShLangFragment;
	case Shader::Type::GEOMETRY: return EShLangGeometry;
	case Shader::Type::COMPUTE: return EShLangCompute;
	case Shader::Type::TESSELATION_CONTROL: return EShLangTessControl;
	case Shader::Type::TESSELATION_EVALUATION: return EShLangTessEvaluation;
	}

	SK_ERROR("Unsupported shader type specified: {0}.", shader_type);
	return EShLanguage(0);
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

VkShaderStageFlagBits Shader::getVulkanType(Type shader_type)
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

glslang::EShTargetClientVersion Shader::getEshClientVersion(APIVersion api_version)
{
	switch (api_version)
	{
	case APIVersion::VULKAN_1_0: return glslang::EShTargetVulkan_1_0;
	case APIVersion::VULKAN_1_1: return glslang::EShTargetVulkan_1_1;
	case APIVersion::VULKAN_1_2: return glslang::EShTargetVulkan_1_2;
	}

	SK_ERROR("Unsupported API version specified: {0}", api_version);
	return glslang::EShTargetClientVersion(0);
}

TBuiltInResource Shader::getResources() 
{
	TBuiltInResource resources = {};
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = Graphics::MAX_IMAGE_SLOTS;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = Graphics::MAX_IMAGE_SLOTS;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.limits.nonInductiveForLoops = true;
	resources.limits.whileLoops = true;
	resources.limits.doWhileLoops = true;
	resources.limits.generalUniformIndexing = true;
	resources.limits.generalAttributeMatrixVectorIndexing = true;
	resources.limits.generalVaryingIndexing = true;
	resources.limits.generalSamplerIndexing = true;
	resources.limits.generalVariableIndexing = true;
	resources.limits.generalConstantMatrixVectorIndexing = true;
	return resources;
}

int32_t Shader::computeSize(const glslang::TType* ttype) 
{
	int32_t components = 0;

	if (ttype->getBasicType() == glslang::EbtStruct || ttype->getBasicType() == glslang::EbtBlock)
		for (const auto& tl : *ttype->getStruct())
			components += computeSize(tl.type);
	else if (ttype->getMatrixCols() != 0)
		components = ttype->getMatrixCols() * ttype->getMatrixRows();
	else
		components = ttype->getVectorSize();

	if (ttype->getArraySizes()) 
	{
		int32_t arraySize = 1;

		for (int32_t d = 0; d < ttype->getArraySizes()->getNumDims(); ++d)
			// This only makes sense in paths that have a known array size.
			if (auto dimSize = ttype->getArraySizes()->getDimSize(d); dimSize != glslang::UnsizedArraySize)
				arraySize *= dimSize;

		components *= arraySize;
	}

	return sizeof(float) * components;
}