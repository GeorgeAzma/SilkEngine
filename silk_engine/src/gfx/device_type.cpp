#include "device_type.h"

VkFormat DeviceTypeInfo::format(DeviceType type)
{
	switch (type)
	{
		case DeviceType::BOOL: return VK_FORMAT_R32_SINT;
		case DeviceType::BYTE: return VK_FORMAT_R8_SINT;
		case DeviceType::UBYTE: return VK_FORMAT_R8_UINT;
		case DeviceType::SHORT: return VK_FORMAT_R16_SINT;
		case DeviceType::USHORT: return VK_FORMAT_R16_UINT;
		case DeviceType::INT: return VK_FORMAT_R32_SINT;
		case DeviceType::UINT: return VK_FORMAT_R32_UINT;
		case DeviceType::FLOAT: return VK_FORMAT_R32_SFLOAT;
		case DeviceType::DOUBLE: return VK_FORMAT_R64_SFLOAT;
		case DeviceType::VEC2: return VK_FORMAT_R32G32_SFLOAT;
		case DeviceType::VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
		case DeviceType::VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DeviceType::IVEC2: return VK_FORMAT_R32G32_SINT;
		case DeviceType::VEC3I: return VK_FORMAT_R32G32B32_SINT;
		case DeviceType::VEC4I: return VK_FORMAT_R32G32B32A32_SINT;
		case DeviceType::VEC2U: return VK_FORMAT_R32G32_UINT;
		case DeviceType::VEC3U: return VK_FORMAT_R32G32B32_UINT;
		case DeviceType::VEC4U: return VK_FORMAT_R32G32B32A32_UINT;
		case DeviceType::VEC2D: return VK_FORMAT_R64G64_SFLOAT;
		case DeviceType::VEC3D: return VK_FORMAT_R64G64B64_SFLOAT;
		case DeviceType::VEC4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case DeviceType::MAT2: return VK_FORMAT_R32G32_SFLOAT;
		case DeviceType::MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
		case DeviceType::MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DeviceType::MAT2D: return VK_FORMAT_R64G64_SFLOAT;
		case DeviceType::MAT3D: return VK_FORMAT_R64G64B64_SFLOAT;
		case DeviceType::MAT4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return VkFormat(0);
}

size_t DeviceTypeInfo::size(DeviceType type)
{
	switch (type)
	{
		case DeviceType::BOOL: return 4;
		case DeviceType::BYTE: return 1;
		case DeviceType::UBYTE: return 1;
		case DeviceType::SHORT: return 2;
		case DeviceType::USHORT: return 2;
		case DeviceType::INT: return 4;
		case DeviceType::UINT: return 4;
		case DeviceType::FLOAT: return 4;
		case DeviceType::DOUBLE: return 8;
		case DeviceType::VEC2: return 8;
		case DeviceType::VEC3: return 12;
		case DeviceType::VEC4: return 16;
		case DeviceType::IVEC2: return 8;
		case DeviceType::VEC3I: return 12;
		case DeviceType::VEC4I: return 16;
		case DeviceType::VEC2U: return 8;
		case DeviceType::VEC3U: return 12;
		case DeviceType::VEC4U: return 16;
		case DeviceType::VEC2D: return 16;
		case DeviceType::VEC3D: return 24;
		case DeviceType::VEC4D: return 32;
		case DeviceType::MAT2: return 16;
		case DeviceType::MAT3: return 36;
		case DeviceType::MAT4: return 64;
		case DeviceType::MAT2D: return 32;
		case DeviceType::MAT3D: return 72;
		case DeviceType::MAT4D: return 128;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}

size_t DeviceTypeInfo::count(DeviceType type)
{
	switch (type)
	{
		case DeviceType::BOOL: return 1;
		case DeviceType::BYTE: return 1;
		case DeviceType::UBYTE: return 1;
		case DeviceType::SHORT: return 1;
		case DeviceType::USHORT: return 1;
		case DeviceType::INT: return 1;
		case DeviceType::UINT: return 1;
		case DeviceType::FLOAT: return 1;
		case DeviceType::DOUBLE: return 1;
		case DeviceType::VEC2: return 2;
		case DeviceType::VEC3: return 3;
		case DeviceType::VEC4: return 4;
		case DeviceType::IVEC2: return 2;
		case DeviceType::VEC3I: return 3;
		case DeviceType::VEC4I: return 4;
		case DeviceType::VEC2U: return 2;
		case DeviceType::VEC3U: return 3;
		case DeviceType::VEC4U: return 4;
		case DeviceType::VEC2D: return 2;
		case DeviceType::VEC3D: return 3;
		case DeviceType::VEC4D: return 4;
		case DeviceType::MAT2: return 4;
		case DeviceType::MAT3: return 9;
		case DeviceType::MAT4: return 16;
		case DeviceType::MAT2D: return 4;
		case DeviceType::MAT3D: return 9;
		case DeviceType::MAT4D: return 16;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}

size_t DeviceTypeInfo::rows(DeviceType type)
{
	switch (type)
	{
		case DeviceType::BOOL: return 1;
		case DeviceType::BYTE: return 1;
		case DeviceType::UBYTE: return 1;
		case DeviceType::SHORT: return 1;
		case DeviceType::USHORT: return 1;
		case DeviceType::INT: return 1;
		case DeviceType::UINT: return 1;
		case DeviceType::FLOAT: return 1;
		case DeviceType::DOUBLE: return 1;
		case DeviceType::VEC2: return 1;
		case DeviceType::VEC3: return 1;
		case DeviceType::VEC4: return 1;
		case DeviceType::IVEC2: return 1;
		case DeviceType::VEC3I: return 1;
		case DeviceType::VEC4I: return 1;
		case DeviceType::VEC2U: return 1;
		case DeviceType::VEC3U: return 1;
		case DeviceType::VEC4U: return 1;
		case DeviceType::VEC2D: return 1;
		case DeviceType::VEC3D: return 1;
		case DeviceType::VEC4D: return 1;
		case DeviceType::MAT2: return 2;
		case DeviceType::MAT3: return 3;
		case DeviceType::MAT4: return 4;
		case DeviceType::MAT2D: return 2;
		case DeviceType::MAT3D: return 3;
		case DeviceType::MAT4D: return 4;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}
