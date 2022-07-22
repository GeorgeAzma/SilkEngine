#pragma once

enum class GpuType
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
	IVEC3,
	IVEC4,
	UVEC2,
	UVEC3,
	UVEC4,
	DVEC2,
	DVEC3,
	DVEC4,
	MAT2,
	MAT3,
	MAT4,
	DMAT2,
	DMAT3,
	DMAT4
};

class GpuTypeEnum
{
public:
	static VkFormat toVulkanType(GpuType type);
	static size_t getSize(GpuType type);
	static size_t getCount(GpuType type);
	static size_t getRows(GpuType type);
};