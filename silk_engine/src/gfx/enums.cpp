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
	case Type::MAT2I: return VK_FORMAT_R32G32_SINT;
	case Type::MAT3I: return VK_FORMAT_R32G32B32_SINT;
	case Type::MAT4I: return VK_FORMAT_R32G32B32A32_SINT;
	case Type::MAT2U: return VK_FORMAT_R32G32_UINT;
	case Type::MAT3U: return VK_FORMAT_R32G32B32_UINT;
	case Type::MAT4U: return VK_FORMAT_R32G32B32A32_UINT;
	case Type::MAT2D: return VK_FORMAT_R64G64_SFLOAT;
	case Type::MAT3D: return VK_FORMAT_R64G64B64_SFLOAT;
	case Type::MAT4D: return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	SK_ERROR("Unsupported type specified: {0}", type);
	return VkFormat(0);
}

Type EnumInfo::formatToType(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_SINT: return Type::BYTE;
	case VK_FORMAT_R8_UINT: return Type::UBYTE;
	case VK_FORMAT_R16_SINT: return Type::SHORT;
	case VK_FORMAT_R16_UINT: return Type::USHORT;
	case VK_FORMAT_R32_SINT: return Type::INT;
	case VK_FORMAT_R32_UINT: return Type::UINT;
	case VK_FORMAT_R32_SFLOAT: return Type::FLOAT;
	case VK_FORMAT_R64_SFLOAT: return Type::DOUBLE;
	case VK_FORMAT_R32G32_SFLOAT: return Type::VEC2;
	case VK_FORMAT_R32G32B32_SFLOAT: return Type::VEC3;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return Type::VEC4;
	case VK_FORMAT_R32G32_SINT: return Type::VEC2I;
	case VK_FORMAT_R32G32B32_SINT: return Type::VEC3I;
	case VK_FORMAT_R32G32B32A32_SINT: return Type::VEC4I;
	case VK_FORMAT_R32G32_UINT: return Type::VEC2U;
	case VK_FORMAT_R32G32B32_UINT:  return Type::VEC3U;
	case VK_FORMAT_R32G32B32A32_UINT: return Type::VEC4U;
	case VK_FORMAT_R64G64_SFLOAT: return Type::VEC2D;
	case VK_FORMAT_R64G64B64_SFLOAT: return Type::VEC3D;
	case VK_FORMAT_R64G64B64A64_SFLOAT: return Type::VEC4D;
	case VK_FORMAT_R8_SRGB: return Type::FLOAT;
	case VK_FORMAT_R8G8_SRGB: return Type::VEC2;
	case VK_FORMAT_R8G8B8_SRGB: return Type::VEC3;
	case VK_FORMAT_R8G8B8A8_SRGB: return Type::VEC4;
	case VK_FORMAT_R8_UNORM: return Type::FLOAT;
	case VK_FORMAT_R8G8_UNORM: return Type::VEC2;
	case VK_FORMAT_R8G8B8_UNORM: return Type::VEC3;
	case VK_FORMAT_R8G8B8A8_UNORM: return Type::VEC4;
	case VK_FORMAT_D16_UNORM: return Type::UINT;
	case VK_FORMAT_D16_UNORM_S8_UINT: return Type::UINT;
	case VK_FORMAT_D24_UNORM_S8_UINT: return Type::UINT;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: return Type::FLOAT;
	case VK_FORMAT_D32_SFLOAT: return Type::FLOAT;
	case VK_FORMAT_B8G8R8_SRGB: return Type::VEC3;
	case VK_FORMAT_B8G8R8A8_SRGB: return Type::VEC4;
	case VK_FORMAT_B8G8R8_UNORM: return Type::VEC3;
	case VK_FORMAT_B8G8R8A8_UNORM: return Type::VEC4;
	}

	SK_ERROR("Unsupported format specified: {0}", format);
	return Type(0);
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
	case Type::MAT2I: return 16;
	case Type::MAT3I: return 36;
	case Type::MAT4I: return 64;
	case Type::MAT2U: return 16;
	case Type::MAT3U: return 36;
	case Type::MAT4U: return 64;
	case Type::MAT2D: return 32;
	case Type::MAT3D: return 72;
	case Type::MAT4D: return 128;
	}

	SK_ERROR("Unsupported type specified: {0}", type);
	return 0;
}

size_t EnumInfo::size(IndexType index_type)
{
	switch (index_type)
	{
	case IndexType::UINT16: return 2;
	case IndexType::UINT32: return 4;
	}

	SK_ERROR("Unsoppurted index type specified: {0}", index_type);
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
	case Type::MAT2I: return 4;
	case Type::MAT3I: return 9;
	case Type::MAT4I: return 16;
	case Type::MAT2U: return 4;
	case Type::MAT3U: return 9;
	case Type::MAT4U: return 16;
	case Type::MAT2D: return 4;
	case Type::MAT3D: return 9;
	case Type::MAT4D: return 16;
	}	
	
	SK_ERROR("Unsupported type specified: {0}", type);
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
	case Type::MAT2I: return 2;
	case Type::MAT3I: return 3;
	case Type::MAT4I: return 4;
	case Type::MAT2U: return 2;
	case Type::MAT3U: return 3;
	case Type::MAT4U: return 4;
	case Type::MAT2D: return 2;
	case Type::MAT3D: return 3;
	case Type::MAT4D: return 4;
	}
	
	SK_ERROR("Unsupported type specified: {0}", type);
	return 0;
}

bool EnumInfo::hasStencil(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT 
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D16_UNORM_S8_UINT
		|| format == VK_FORMAT_S8_UINT;
}

bool EnumInfo::hasDepth(VkFormat format)
{
	return ((format == VK_FORMAT_D32_SFLOAT) || hasStencil(format));
}

size_t EnumInfo::channelCount(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_SINT: return 1;
	case VK_FORMAT_R8_UINT: return 1;
	case VK_FORMAT_R16_SINT: return 1;
	case VK_FORMAT_R16_UINT: return 1;
	case VK_FORMAT_R32_SINT: return 1;
	case VK_FORMAT_R32_UINT: return 1;
	case VK_FORMAT_R32_SFLOAT: return 1;
	case VK_FORMAT_R64_SFLOAT: return 1;
	case VK_FORMAT_R32G32_SFLOAT: return 2;
	case VK_FORMAT_R32G32B32_SFLOAT: return 3;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
	case VK_FORMAT_R32G32_SINT: return 2;
	case VK_FORMAT_R32G32B32_SINT: return 3;
	case VK_FORMAT_R32G32B32A32_SINT: return 4;
	case VK_FORMAT_R32G32_UINT: return 2;
	case VK_FORMAT_R32G32B32_UINT:  return 3;
	case VK_FORMAT_R32G32B32A32_UINT: return 4;
	case VK_FORMAT_R64G64_SFLOAT: return 2;
	case VK_FORMAT_R64G64B64_SFLOAT: return 3;
	case VK_FORMAT_R64G64B64A64_SFLOAT: return 4;
	case VK_FORMAT_R8_SRGB: return 1;
	case VK_FORMAT_R8G8_SRGB: return 2;
	case VK_FORMAT_R8G8B8_SRGB: return 3;
	case VK_FORMAT_R8G8B8A8_SRGB: return 4;
	case VK_FORMAT_R8_UNORM: return 1;
	case VK_FORMAT_R8G8_UNORM: return 2;
	case VK_FORMAT_R8G8B8_UNORM: return 3;
	case VK_FORMAT_R8G8B8A8_UNORM: return 4;
	case VK_FORMAT_D16_UNORM: return 1;
	case VK_FORMAT_D16_UNORM_S8_UINT: return 1;
	case VK_FORMAT_D24_UNORM_S8_UINT: return 1;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: return 1;
	case VK_FORMAT_D32_SFLOAT: return 1;
	case VK_FORMAT_B8G8R8_SRGB: return 3;
	case VK_FORMAT_B8G8R8A8_SRGB: return 4;
	case VK_FORMAT_B8G8R8_UNORM: return 3;
	case VK_FORMAT_B8G8R8A8_UNORM: return 4;
	}

	SK_ERROR("Unsupported format specified: {0}", format);
	return 0;
}

size_t EnumInfo::formatSize(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_SINT: return 1;
	case VK_FORMAT_R8_UINT: return 1;
	case VK_FORMAT_R16_SINT: return 2;
	case VK_FORMAT_R16_UINT: return 2;
	case VK_FORMAT_R32_SINT: return 4;
	case VK_FORMAT_R32_UINT: return 4;
	case VK_FORMAT_R32_SFLOAT: return 4;
	case VK_FORMAT_R64_SFLOAT: return 8;
	case VK_FORMAT_R32G32_SFLOAT: return 8;
	case VK_FORMAT_R32G32B32_SFLOAT: return 12;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
	case VK_FORMAT_R32G32_SINT: return 8;
	case VK_FORMAT_R32G32B32_SINT: return 12;
	case VK_FORMAT_R32G32B32A32_SINT: return 16;
	case VK_FORMAT_R32G32_UINT: return 8;
	case VK_FORMAT_R32G32B32_UINT:  return 12;
	case VK_FORMAT_R32G32B32A32_UINT: return 16;
	case VK_FORMAT_R64G64_SFLOAT: return 128;
	case VK_FORMAT_R64G64B64_SFLOAT: return 192;
	case VK_FORMAT_R64G64B64A64_SFLOAT: return 256;
	case VK_FORMAT_R8_SRGB: return 1;
	case VK_FORMAT_R8G8_SRGB: return 2;
	case VK_FORMAT_R8G8B8_SRGB: return 3;
	case VK_FORMAT_R8G8B8A8_SRGB: return 4;
	case VK_FORMAT_R8_UNORM: return 1;
	case VK_FORMAT_R8G8_UNORM: return 2;
	case VK_FORMAT_R8G8B8_UNORM: return 3;
	case VK_FORMAT_R8G8B8A8_UNORM: return 4;
	case VK_FORMAT_D16_UNORM: return 2;
	case VK_FORMAT_D16_UNORM_S8_UINT: return 3;
	case VK_FORMAT_D24_UNORM_S8_UINT: return 4;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: return 5;
	case VK_FORMAT_D32_SFLOAT: return 4;
	case VK_FORMAT_B8G8R8_SRGB: return 3;
	case VK_FORMAT_B8G8R8A8_SRGB: return 4;
	case VK_FORMAT_B8G8R8_UNORM: return 3;
	case VK_FORMAT_B8G8R8A8_UNORM: return 4;
	}

	SK_ERROR("Unsupported format specified: {0}", format);
	return 0;
}

VkImageAspectFlags EnumInfo::getAspectFlags(VkFormat format)
{
	if (!EnumInfo::hasDepth(format))
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else
	{
		if (EnumInfo::hasStencil(format))
		{
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
}

VkIndexType EnumInfo::indexType(IndexType index_type)
{
	switch (index_type)
	{
	case IndexType::UINT16: return VK_INDEX_TYPE_UINT16;
	case IndexType::UINT32: return VK_INDEX_TYPE_UINT32;
	}

	SK_ERROR("Unsupported index type specified: {0}", index_type);
	return VkIndexType(0);
}

VkShaderStageFlagBits EnumInfo::shaderType(ShaderType shader_type)
{
	switch (shader_type)
	{
	case ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderType::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderType::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
	case ShaderType::TESSELATION_CONTROL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderType::TESSELATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	SK_ERROR("Unsupported shader type specified: {0}. try renaming shader file extensions to: .vert .frag .geom .tcs .tes", shader_type);
	return VkShaderStageFlagBits(0);
}
