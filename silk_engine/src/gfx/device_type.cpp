#include "device_type.h"

VkFormat DeviceTypeEnum::toVulkanType(DeviceType type)
{
	using enum DeviceType;
	switch (type)
	{
		case BOOL: return VK_FORMAT_R32_SINT;
		case BYTE: return VK_FORMAT_R8_SINT;
		case UBYTE: return VK_FORMAT_R8_UINT;
		case SHORT: return VK_FORMAT_R16_SINT;
		case USHORT: return VK_FORMAT_R16_UINT;
		case INT: return VK_FORMAT_R32_SINT;
		case UINT: return VK_FORMAT_R32_UINT;
		case FLOAT: return VK_FORMAT_R32_SFLOAT;
		case DOUBLE: return VK_FORMAT_R64_SFLOAT;
		case VEC2: return VK_FORMAT_R32G32_SFLOAT;
		case VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
		case VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case IVEC2: return VK_FORMAT_R32G32_SINT;
		case VEC3I: return VK_FORMAT_R32G32B32_SINT;
		case VEC4I: return VK_FORMAT_R32G32B32A32_SINT;
		case VEC2U: return VK_FORMAT_R32G32_UINT;
		case VEC3U: return VK_FORMAT_R32G32B32_UINT;
		case VEC4U: return VK_FORMAT_R32G32B32A32_UINT;
		case VEC2D: return VK_FORMAT_R64G64_SFLOAT;
		case VEC3D: return VK_FORMAT_R64G64B64_SFLOAT;
		case VEC4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case MAT2: return VK_FORMAT_R32G32_SFLOAT;
		case MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
		case MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case MAT2D: return VK_FORMAT_R64G64_SFLOAT;
		case MAT3D: return VK_FORMAT_R64G64B64_SFLOAT;
		case MAT4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	SK_ERROR("Unsupported device type specified: {0}.", type);
	return VkFormat(0);
}

size_t DeviceTypeEnum::getSize(DeviceType type)
{
	using enum DeviceType;
	switch (type)
	{
		case BOOL: return 4;
		case BYTE: return 1;
		case UBYTE: return 1;
		case SHORT: return 2;
		case USHORT: return 2;
		case INT: return 4;
		case UINT: return 4;
		case FLOAT: return 4;
		case DOUBLE: return 8;
		case VEC2: return 8;
		case VEC3: return 12;
		case VEC4: return 16;
		case IVEC2: return 8;
		case VEC3I: return 12;
		case VEC4I: return 16;
		case VEC2U: return 8;
		case VEC3U: return 12;
		case VEC4U: return 16;
		case VEC2D: return 16;
		case VEC3D: return 24;
		case VEC4D: return 32;
		case MAT2: return 16;
		case MAT3: return 36;
		case MAT4: return 64;
		case MAT2D: return 32;
		case MAT3D: return 72;
		case MAT4D: return 128;
	}

	SK_ERROR("Unsupported device type specified: {0}.", type);
	return 0;
}

size_t DeviceTypeEnum::getCount(DeviceType type)
{
	using enum DeviceType;
	switch (type)
	{
		case BOOL: return 1;
		case BYTE: return 1;
		case UBYTE: return 1;
		case SHORT: return 1;
		case USHORT: return 1;
		case INT: return 1;
		case UINT: return 1;
		case FLOAT: return 1;
		case DOUBLE: return 1;
		case VEC2: return 2;
		case VEC3: return 3;
		case VEC4: return 4;
		case IVEC2: return 2;
		case VEC3I: return 3;
		case VEC4I: return 4;
		case VEC2U: return 2;
		case VEC3U: return 3;
		case VEC4U: return 4;
		case VEC2D: return 2;
		case VEC3D: return 3;
		case VEC4D: return 4;
		case MAT2: return 4;
		case MAT3: return 9;
		case MAT4: return 16;
		case MAT2D: return 4;
		case MAT3D: return 9;
		case MAT4D: return 16;
	}

	SK_ERROR("Unsupported device type specified: {0}.", type);
	return 0;
}

size_t DeviceTypeEnum::getRows(DeviceType type)
{
	using enum DeviceType;
	switch (type)
	{
		case BOOL: return 1;
		case BYTE: return 1;
		case UBYTE: return 1;
		case SHORT: return 1;
		case USHORT: return 1;
		case INT: return 1;
		case UINT: return 1;
		case FLOAT: return 1;
		case DOUBLE: return 1;
		case VEC2: return 1;
		case VEC3: return 1;
		case VEC4: return 1;
		case IVEC2: return 1;
		case VEC3I: return 1;
		case VEC4I: return 1;
		case VEC2U: return 1;
		case VEC3U: return 1;
		case VEC4U: return 1;
		case VEC2D: return 1;
		case VEC3D: return 1;
		case VEC4D: return 1;
		case MAT2: return 2;
		case MAT3: return 3;
		case MAT4: return 4;
		case MAT2D: return 2;
		case MAT3D: return 3;
		case MAT4D: return 4;
	}

	SK_ERROR("Unsupported device type specified: {0}.", type);
	return 0;
}
