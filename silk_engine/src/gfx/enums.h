#pragma once

#include <shaderc/shaderc.h>

enum class Type
{
	BOOL,
	BYTE,
	UBYTE,
	SHORT,
	USHORT,
	INT,
	UINT,
	FLOAT,
	DOUBLE,
	VEC2,
	VEC3,
	VEC4,
	VEC2I,
	VEC3I,
	VEC4I,
	VEC2U,
	VEC3U,
	VEC4U,
	VEC2D,
	VEC3D,
	VEC4D,
	MAT2,
	MAT3,
	MAT4,
	MAT2I,
	MAT3I,
	MAT4I,
	MAT2U,
	MAT3U,
	MAT4U,
	MAT2D,
	MAT3D,
	MAT4D
};

enum class IndexType
{
	UINT16,
	UINT32
};

enum class ShaderType : uint32_t
{
	NONE = 0,
	VERTEX = 1,
	FRAGMENT = 2,
	GEOMETRY = 4,
	COMPUTE = 8,
	TESSELATION_CONTROL = 16,
	TESSELATION_EVALUATION = 32
};

enum class EnableTag
{
	DEPTH_TEST,
	DEPTH_WRITE,
	STENCIL_TEST,
	COLOR_BLENDING,
	SAMPLE_SHADING,
	PRIMITISK_RESTART,
	RASTERIZER_DISCARD,
	DEPTH_CLAMP,
	DEPTH_BIAS,
	COLOR_BLEND_LOGIC_OP
};

enum class APIVersion
{
	VULKAN_1_0,
	VULKAN_1_1,
	VULKAN_1_2,
};

class EnumInfo
{
public:
	static VkFormat type(Type type);
	static Type formatToType(VkFormat format);
	static VkIndexType indexType(IndexType index_type);
	static VkShaderStageFlagBits shaderType(ShaderType shader_type);
	static std::string shaderTypeFileExtension(ShaderType shader_type);
	static shaderc_shader_kind shadercType(ShaderType shader_type);
	static ShaderType shaderString(const std::string& shader_string);
	static uint32_t apiVersion(APIVersion api_version);
	static shaderc_env_version shadercApiVersion(APIVersion api_version);
	static size_t size(Type type);
	static size_t size(IndexType index_type);
	static size_t count(Type type);
	static size_t rows(Type type);
	static size_t channelCount(VkFormat format);
	static size_t formatSize(VkFormat format);
};