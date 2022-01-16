#pragma once

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

enum class ShaderType
{
	NONE,
	VERTEX,
	FRAGMENT,
	GEOMETRY,
	COMPUTE,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION
};

enum class EnableTag
{
	DEPTH_TEST,
	DEPTH_WRITE,
	STENCIL_TEST,
	COLOR_BLENDING,
	SAMPLE_SHADING,
	PRIMITIVE_RESTART,
	RASTERIZER_DISCARD,
	DEPTH_CLAMP,
	DEPTH_BIAS,
	COLOR_BLEND_LOGIC_OP
};

class EnumInfo
{
public:
	static VkFormat type(Type type);
	static Type formatToType(VkFormat format);
	static VkIndexType indexType(IndexType index_type);
	static VkShaderStageFlagBits shaderType(ShaderType shader_type);
	static size_t size(Type type);
	static size_t size(IndexType index_type);
	static size_t count(Type type);
	static size_t rows(Type type);
	static bool hasStencil(VkFormat format);
	static bool hasDepth(VkFormat format);
	static VkImageAspectFlags getAspectFlags(VkFormat format);
};