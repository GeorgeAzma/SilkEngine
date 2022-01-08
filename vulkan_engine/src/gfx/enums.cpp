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

	VE_CORE_ERROR("Unsupported type specified: {0}", type);
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

	VE_CORE_ERROR("Unsupported type specified: {0}", type);
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
	case Type::DOUBLE: return 1; //TODO: It might be better to write 2x for doubles
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
	
	VE_CORE_ERROR("Unsupported type specified: {0}", type);
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
	case Type::VEC2D: return 1; //TODO: Probably 2x for double here too
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
	
	VE_CORE_ERROR("Unsupported type specified: {0}", type);
	return 0;
}

VkIndexType EnumInfo::indexType(IndexType index_type)
{
	switch (index_type)
	{
	case IndexType::UINT16: return VK_INDEX_TYPE_UINT16;
	case IndexType::UINT32: return VK_INDEX_TYPE_UINT32;
	}

	VE_CORE_ERROR("Unsupported index type specified: {0}", index_type);
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

	VE_CORE_ERROR("Unsupported shader type specified: {0}. try renaming shader file extensions to: .vert .frag .geom .tcs .tes", shader_type);
	return VkShaderStageFlagBits(0);
}
