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

Shader::Shader(const std::string& file, const std::vector<Define>& defines)
	: file(file)
{
	compile(defines);
}

void Shader::compile(const std::vector<Define>& defines)
{
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

	std::string path = std::string("data/shaders/") + file + ".glsl";
	std::string cache_path = std::string("data/cache/shaders/") + file + ".glsl";
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
		
		std::string file_cache_path = cache_path + getTypeFileExtension((Type)type) + ".spv";
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

		//Reflection
		program.buildReflection();	
		
		//Local size reflection
		for (uint32_t dimension = 0; dimension < 3; ++dimension)
			if (auto local_size = program.getLocalSize(dimension); local_size > 1)
				local_sizes[dimension] = local_size;
		
		for (int32_t i = program.getNumLiveUniformBlocks() - 1; i >= 0; --i)
			loadUniformBlock(program, stage_flag, i);
		
		for (size_t i = 0; i < program.getNumLiveUniformVariables(); ++i)
			loadUniform(program, stage_flag, i);
		
		for (size_t i = 0; i < program.getNumLiveAttributes(); ++i)
			loadAttribute(program, i);

		pipeline_shader_stage_infos.emplace_back(std::move(pipeline_shader_stage_info));
	}

	for (const auto& [uniform_block_name, uniform_block] : uniform_blocks)
	{
		auto descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		switch (uniform_block.type)
		{
		case UniformBlock::Type::Uniform:
			descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case UniformBlock::Type::Storage:
			descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		case UniformBlock::Type::Push:
			break;
		}
		//TODO:
		//shared<DescriptorSetLayout> descriptor_set_layout = makeShared<DescriptorSetLayout>();
		//descriptor_set_layout->addBinding(uniform_block.binding, descriptor_type, uniform_block.stage_flags)
		//	.build();
	}
	
	for (const auto& [uniform_name, uniform] : uniforms)
	{
		auto descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	
		switch (uniform.gl_type)
		{
		case 0x8B5E: // GL_SAMPLER_2D
		case 0x904D: // GL_IMAGE_2D
		case 0x8DC1: // GL_TEXTURE_2D_ARRAY
		case 0x9108: // GL_SAMPLER_2D_MULTISAMPLE
		case 0x9055: // GL_IMAGE_2D_MULTISAMPLE
			descriptor_type = uniform.write_only ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		case 0x8B60: // GL_SAMPLER_CUBE
		case 0x9050: // GL_IMAGE_CUBE
		case 0x9054: // GL_IMAGE_CUBE_MAP_ARRAY
			descriptor_type = uniform.write_only ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		//TODO:
		//shared<DescriptorSetLayout> descriptor_set_layout = makeShared<DescriptorSetLayout>();
		//descriptor_set_layout->addBinding(uniform.binding, descriptor_type, uniform.stage_flags)
		//	.build();
	}
	
	//TODO: This is currently useless until I it supports instancing
	uint32_t current_offset = 4;
	for (const auto& [attribute_name, attribute] : attributes) 
	{
		VkVertexInputAttributeDescription attribute_description = {};
		attribute_description.location = uint32_t(attribute.location);
		attribute_description.binding = 0;
		attribute_description.format = EnumInfo::glTypeToVk(attribute.gl_type);
		attribute_description.offset = current_offset;
		attribute_descriptions.emplace_back(std::move(attribute_description));
		current_offset += attribute.size;
	}

	SK_TRACE("Shader loaded: {0}", path);
}

Shader::~Shader()
{
	for(auto& pipeline_shader_stage_info : pipeline_shader_stage_infos)
		vkDestroyShaderModule(*Graphics::logical_device, pipeline_shader_stage_info.module, nullptr);
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

void Shader::loadUniformBlock(const glslang::TProgram& program, VkShaderStageFlagBits stage_flag, int32_t index)
{
	auto reflection = program.getUniformBlock(index);
	for (auto& [uniform_block_name, uniform_block] : uniform_blocks)
	{
		if (uniform_block_name == reflection.name)
		{
			uniform_block.stage_flags |= stage_flag;
			return;
		}
	}

	auto uniform_type = UniformBlock::Type::None;
	if (reflection.getType()->getQualifier().storage == glslang::EvqUniform)
		uniform_type = UniformBlock::Type::Uniform;
	if (reflection.getType()->getQualifier().storage == glslang::EvqBuffer)
		uniform_type = UniformBlock::Type::Storage;
	if (reflection.getType()->getQualifier().layoutPushConstant)
		uniform_type = UniformBlock::Type::Push;

	uniform_blocks.emplace(reflection.name, UniformBlock(reflection.getBinding(), reflection.size, stage_flag, uniform_type));
}

void Shader::loadUniform(const glslang::TProgram& program, VkShaderStageFlagBits stage_flag, int32_t index)
{
	auto reflection = program.getUniform(index);

	if (reflection.getBinding() == -1)
	{
		auto split_name = String::split(reflection.name, '.');
		if (split_name.size() > 1)
		{
			for (auto& [uniform_block_name, uniform_block] : uniform_blocks)
			{
				if (uniform_block_name == split_name.at(0))
				{
					uniform_block.uniforms.emplace(String::replaceFirst(reflection.name, split_name.at(0) + ".", ""),
						Uniform(reflection.getBinding(), reflection.offset, computeSize(reflection.getType()), reflection.glDefineType, false, false, stage_flag));
					return;
				}
			}
		}
	}

	for (auto& [uniform_name, uniform] : uniforms)
	{
		if (uniform_name == reflection.name)
		{
			uniform.stage_flags |= stage_flag;
			return;
		}
	}

	auto& qualifier = reflection.getType()->getQualifier();
	uniforms.emplace(reflection.name, Uniform(reflection.getBinding(), reflection.offset, -1, reflection.glDefineType, qualifier.readonly, qualifier.writeonly, stage_flag));
}

void Shader::loadAttribute(const glslang::TProgram& program, int32_t index)
{
	auto reflection = program.getPipeInput(index);

	if (reflection.name.empty())
		return;

	for (const auto& [attribute_name, attribute] : attributes)
		if (attribute_name == reflection.name)
			return;

	auto& qualifier = reflection.getType()->getQualifier();
	attributes.emplace(reflection.name, Attribute(qualifier.layoutSet, qualifier.layoutLocation, computeSize(reflection.getType()), reflection.glDefineType));
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