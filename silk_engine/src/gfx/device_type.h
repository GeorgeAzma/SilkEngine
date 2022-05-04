#pragma once

enum class DeviceType
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

class DeviceTypeEnum
{
public:
	static VkFormat toVulkanType(DeviceType type);
	static size_t getSize(DeviceType type);
	static size_t getCount(DeviceType type);
	static size_t getRows(DeviceType type);
};