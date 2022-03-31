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
	IVEC2,
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
	MAT2D,
	MAT3D,
	MAT4D
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
	static VkFormat glTypeToVk(uint32_t gl_type);
	static uint32_t apiVersion(APIVersion api_version);
	static size_t size(Type type);
	static size_t count(Type type);
	static size_t rows(Type type);
	static bool needsStaging(VmaMemoryUsage usage);
	static std::string stringifyResult(VkResult result);
};