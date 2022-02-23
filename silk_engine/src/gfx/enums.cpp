#include "enums.h"

VkFormat EnumInfo::type(Type type)
{
	switch (type)
	{
	case Type::BOOL: return VK_FORMAT_R32_SINT;
	case Type::BYTE: return VK_FORMAT_R8_SINT;
	case Type::UBYTE: return VK_FORMAT_R8_UINT;
	case Type::SHORT: return VK_FORMAT_R16_SINT;
	case Type::USHORT: return VK_FORMAT_R16_UINT;
	case Type::INT: return VK_FORMAT_R32_SINT;
	case Type::UINT: return VK_FORMAT_R32_UINT;
	case Type::FLOAT: return VK_FORMAT_R32_SFLOAT;
	case Type::DOUBLE: return VK_FORMAT_R64_SFLOAT;
	case Type::VEC2: return VK_FORMAT_R32G32_SFLOAT;
	case Type::VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
	case Type::VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Type::VEC2I: return VK_FORMAT_R32G32_SINT;
	case Type::VEC3I: return VK_FORMAT_R32G32B32_SINT;
	case Type::VEC4I: return VK_FORMAT_R32G32B32A32_SINT;
	case Type::VEC2U: return VK_FORMAT_R32G32_UINT;
	case Type::VEC3U: return VK_FORMAT_R32G32B32_UINT;
	case Type::VEC4U: return VK_FORMAT_R32G32B32A32_UINT;
	case Type::VEC2D: return VK_FORMAT_R64G64_SFLOAT;
	case Type::VEC3D: return VK_FORMAT_R64G64B64_SFLOAT;
	case Type::VEC4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
	case Type::MAT2: return VK_FORMAT_R32G32_SFLOAT;
	case Type::MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
	case Type::MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Type::MAT2D: return VK_FORMAT_R64G64_SFLOAT;
	case Type::MAT3D: return VK_FORMAT_R64G64B64_SFLOAT;
	case Type::MAT4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return VkFormat(0);
}

VkFormat EnumInfo::glTypeToVk(uint32_t gl_type)
{
	switch (gl_type)
	{
	case 0x8B56: return VK_FORMAT_R32_SINT;
	case 0x1400: return VK_FORMAT_R8_SINT;
	case 0x1401: return VK_FORMAT_R8_UINT;
	case 0x1402: return VK_FORMAT_R16_SINT;
	case 0x1403: return VK_FORMAT_R16_UINT;
	case 0x1404: return VK_FORMAT_R32_SINT;
	case 0x1405: return VK_FORMAT_R32_UINT;
	case 0x1406: return VK_FORMAT_R32_SFLOAT;
	case 0x140A: return VK_FORMAT_R64_SFLOAT;
	case 0x8B50: return VK_FORMAT_R32G32_SFLOAT;
	case 0x8B51: return VK_FORMAT_R32G32B32_SFLOAT;
	case 0x8B52: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case 0x8B53: return VK_FORMAT_R32G32_SINT;
	case 0x8B54: return VK_FORMAT_R32G32B32_SINT;
	case 0x8B55: return VK_FORMAT_R32G32B32A32_SINT;
	case 0x8DC6: return VK_FORMAT_R32G32_UINT;
	case 0x8DC7: return VK_FORMAT_R32G32B32_UINT;
	case 0x8DC8: return VK_FORMAT_R32G32B32A32_UINT;
	case 0x8FFC: return VK_FORMAT_R64G64_SFLOAT;
	case 0x8FFD: return VK_FORMAT_R64G64B64_SFLOAT;
	case 0x8FFE: return VK_FORMAT_R64G64B64A64_SFLOAT;
	case 0x8B5A: return VK_FORMAT_R32G32_SFLOAT;
	case 0x8B5B: return VK_FORMAT_R32G32B32_SFLOAT;
	case 0x8B5C: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case 0x8F46: return VK_FORMAT_R64G64_SFLOAT;
	case 0x8F47: return VK_FORMAT_R64G64B64_SFLOAT;
	case 0x8F48: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	SK_ERROR("Unsupported gl_type specified: {0}.", gl_type);
	return VkFormat(0);
}

size_t EnumInfo::size(Type type)
{
	switch (type)
	{
	case Type::BOOL: return 4;
	case Type::BYTE: return 1;
	case Type::UBYTE: return 1;
	case Type::SHORT: return 2;
	case Type::USHORT: return 2;
	case Type::INT: return 4;
	case Type::UINT: return 4;
	case Type::FLOAT: return 4;
	case Type::DOUBLE: return 8;
	case Type::VEC2: return 8;
	case Type::VEC3: return 12;
	case Type::VEC4: return 16;
	case Type::VEC2I: return 8;
	case Type::VEC3I: return 12;
	case Type::VEC4I: return 16;
	case Type::VEC2U: return 8;
	case Type::VEC3U: return 12;
	case Type::VEC4U: return 16;
	case Type::VEC2D: return 16;
	case Type::VEC3D: return 24;
	case Type::VEC4D: return 32;
	case Type::MAT2: return 16;
	case Type::MAT3: return 36;
	case Type::MAT4: return 64;
	case Type::MAT2D: return 32;
	case Type::MAT3D: return 72;
	case Type::MAT4D: return 128;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}

size_t EnumInfo::size(IndexType index_type)
{
	switch (index_type)
	{
	case IndexType::UINT16: return 2;
	case IndexType::UINT32: return 4;
	}

	SK_ERROR("Unsoppurted index type specified: {0}.", index_type);
	return 0;
}

size_t EnumInfo::count(Type type)
{
	switch (type)
	{
	case Type::BOOL: return 1;
	case Type::BYTE: return 1;
	case Type::UBYTE: return 1;
	case Type::SHORT: return 1;
	case Type::USHORT: return 1;
	case Type::INT: return 1;
	case Type::UINT: return 1;
	case Type::FLOAT: return 1;
	case Type::DOUBLE: return 1;
	case Type::VEC2: return 2;
	case Type::VEC3: return 3;
	case Type::VEC4: return 4;
	case Type::VEC2I: return 2;
	case Type::VEC3I: return 3;
	case Type::VEC4I: return 4;
	case Type::VEC2U: return 2;
	case Type::VEC3U: return 3;
	case Type::VEC4U: return 4;
	case Type::VEC2D: return 2;
	case Type::VEC3D: return 3;
	case Type::VEC4D: return 4;
	case Type::MAT2: return 4;
	case Type::MAT3: return 9;
	case Type::MAT4: return 16;
	case Type::MAT2D: return 4;
	case Type::MAT3D: return 9;
	case Type::MAT4D: return 16;
	}	
	
	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}

size_t EnumInfo::rows(Type type)
{
	switch (type)
	{
	case Type::BOOL: return 1;
	case Type::BYTE: return 1;
	case Type::UBYTE: return 1;
	case Type::SHORT: return 1;
	case Type::USHORT: return 1;
	case Type::INT: return 1;
	case Type::UINT: return 1;
	case Type::FLOAT: return 1;
	case Type::DOUBLE: return 1;
	case Type::VEC2: return 1;
	case Type::VEC3: return 1;
	case Type::VEC4: return 1;
	case Type::VEC2I: return 1;
	case Type::VEC3I: return 1;
	case Type::VEC4I: return 1;
	case Type::VEC2U: return 1;
	case Type::VEC3U: return 1;
	case Type::VEC4U: return 1;
	case Type::VEC2D: return 1;
	case Type::VEC3D: return 1;
	case Type::VEC4D: return 1;
	case Type::MAT2: return 2;
	case Type::MAT3: return 3;
	case Type::MAT4: return 4;
	case Type::MAT2D: return 2;
	case Type::MAT3D: return 3;
	case Type::MAT4D: return 4;
	}
	
	SK_ERROR("Unsupported type specified: {0}.", type);
	return 0;
}

bool EnumInfo::needsStaging(VmaMemoryUsage usage)
{
	switch (usage)
	{
	case VMA_MEMORY_USAGE_AUTO:
	case VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE:
	case VMA_MEMORY_USAGE_CPU_COPY:
	case VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED:
	case VMA_MEMORY_USAGE_GPU_ONLY:
		return true;
	}

	return false;
}

VkIndexType EnumInfo::indexType(IndexType index_type)
{
	switch (index_type)
	{
	case IndexType::UINT16: return VK_INDEX_TYPE_UINT16;
	case IndexType::UINT32: return VK_INDEX_TYPE_UINT32;
	}

	SK_ERROR("Unsupported index type specified: {0}.", index_type);
	return VkIndexType(0);
}

uint32_t EnumInfo::apiVersion(APIVersion api_version)
{
	switch (api_version)
	{
	case APIVersion::VULKAN_1_0: return VK_API_VERSION_1_0;
	case APIVersion::VULKAN_1_1: return VK_API_VERSION_1_1;
	case APIVersion::VULKAN_1_2: return VK_API_VERSION_1_2;
	}

	SK_ERROR("Unsupported api version specified: {0}.", api_version);
	return uint32_t(0);
}