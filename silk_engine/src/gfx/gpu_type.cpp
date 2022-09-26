#include "gpu_type.h"

VkFormat GpuTypeEnum::toVulkanType(GpuType type)
{
	using enum GpuType;
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
		case IVEC3: return VK_FORMAT_R32G32B32_SINT;
		case IVEC4: return VK_FORMAT_R32G32B32A32_SINT;
		case UVEC2: return VK_FORMAT_R32G32_UINT;
		case UVEC3: return VK_FORMAT_R32G32B32_UINT;
		case UVEC4: return VK_FORMAT_R32G32B32A32_UINT;
		case DVEC2: return VK_FORMAT_R64G64_SFLOAT;
		case DVEC3: return VK_FORMAT_R64G64B64_SFLOAT;
		case DVEC4: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case MAT2: return VK_FORMAT_R32G32_SFLOAT;
		case MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
		case MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DMAT2: return VK_FORMAT_R64G64_SFLOAT;
		case DMAT3: return VK_FORMAT_R64G64B64_SFLOAT;
		case DMAT4: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	return VkFormat(0);
}

size_t GpuTypeEnum::getSize(GpuType type)
{
	using enum GpuType;
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
		case IVEC3: return 12;
		case IVEC4: return 16;
		case UVEC2: return 8;
		case UVEC3: return 12;
		case UVEC4: return 16;
		case DVEC2: return 16;
		case DVEC3: return 24;
		case DVEC4: return 32;
		case MAT2: return 16;
		case MAT3: return 36;
		case MAT4: return 64;
		case DMAT2: return 32;
		case DMAT3: return 72;
		case DMAT4: return 128;
	}

	return 0;
}

size_t GpuTypeEnum::getCount(GpuType type)
{
	using enum GpuType;
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
		case IVEC3: return 3;
		case IVEC4: return 4;
		case UVEC2: return 2;
		case UVEC3: return 3;
		case UVEC4: return 4;
		case DVEC2: return 2;
		case DVEC3: return 3;
		case DVEC4: return 4;
		case MAT2: return 4;
		case MAT3: return 9;
		case MAT4: return 16;
		case DMAT2: return 4;
		case DMAT3: return 9;
		case DMAT4: return 16;
	}

	return 0;
}

size_t GpuTypeEnum::getRows(GpuType type)
{
	using enum GpuType;
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
		case IVEC3: return 1;
		case IVEC4: return 1;
		case UVEC2: return 1;
		case UVEC3: return 1;
		case UVEC4: return 1;
		case DVEC2: return 1;
		case DVEC3: return 1;
		case DVEC4: return 1;
		case MAT2: return 2;
		case MAT3: return 3;
		case MAT4: return 4;
		case DMAT2: return 2;
		case DMAT3: return 3;
		case DMAT4: return 4;
	}

	return 0;
}
