#include "enums.h"

vk::Format EnumInfo::type(Type type)
{
	using enum vk::Format;
	switch (type)
	{
	case Type::BOOL: return eR32Sint;
	case Type::BYTE: return eR8Sint;
	case Type::UBYTE: return eR8Uint;
	case Type::SHORT: return eR16Sint;
	case Type::USHORT: return eR16Uint;
	case Type::INT: return eR32Sint;
	case Type::UINT: return eR32Uint;
	case Type::FLOAT: return eR32Sfloat;
	case Type::DOUBLE: return eR64Sfloat;
	case Type::VEC2: return eR32G32Sfloat;
	case Type::VEC3: return eR32G32B32Sfloat;
	case Type::VEC4: return eR32G32B32A32Sfloat;
	case Type::VEC2I: return eR32G32Sint;
	case Type::VEC3I: return eR32G32B32Sint;
	case Type::VEC4I: return eR32G32B32A32Sint;
	case Type::VEC2U: return eR32G32Uint;
	case Type::VEC3U: return eR32G32B32Uint;
	case Type::VEC4U: return eR32G32B32A32Uint;
	case Type::VEC2D: return eR64G64Sfloat;
	case Type::VEC3D: return eR64G64B64Sfloat;
	case Type::VEC4D: return eR64G64B64A64Sfloat;
	case Type::MAT2: return eR32G32Sfloat;
	case Type::MAT3: return eR32G32B32Sfloat;
	case Type::MAT4: return eR32G32B32A32Sfloat;
	case Type::MAT2D: return eR64G64Sfloat;
	case Type::MAT3D: return eR64G64B64Sfloat;
	case Type::MAT4D: return eR64G64B64A64Sfloat;
	}

	SK_ERROR("Unsupported type specified: {0}.", type);
	return vk::Format(0);
}

vk::Format EnumInfo::glTypeToVk(uint32_t gl_type)
{
	using enum vk::Format;
	switch (gl_type)
	{
	case 0x8B56: return eR32Sint;
	case 0x1400: return eR8Sint;
	case 0x1401: return eR8Uint;
	case 0x1402: return eR16Sint;
	case 0x1403: return eR16Uint;
	case 0x1404: return eR32Sint;
	case 0x1405: return eR32Uint;
	case 0x1406: return eR32Sfloat;
	case 0x140A: return eR64Sfloat;
	case 0x8B50: return eR32G32Sfloat;
	case 0x8B51: return eR32G32B32Sfloat;
	case 0x8B52: return eR32G32B32A32Sfloat;
	case 0x8B53: return eR32G32Sint;
	case 0x8B54: return eR32G32B32Sint;
	case 0x8B55: return eR32G32B32A32Sint;
	case 0x8DC6: return eR32G32Uint;
	case 0x8DC7: return eR32G32B32Uint;
	case 0x8DC8: return eR32G32B32A32Uint;
	case 0x8FFC: return eR64G64Sfloat;
	case 0x8FFD: return eR64G64B64Sfloat;
	case 0x8FFE: return eR64G64B64A64Sfloat;
	case 0x8B5A: return eR32G32Sfloat;
	case 0x8B5B: return eR32G32B32Sfloat;
	case 0x8B5C: return eR32G32B32A32Sfloat;
	case 0x8F46: return eR64G64Sfloat;
	case 0x8F47: return eR64G64B64Sfloat;
	case 0x8F48: return eR64G64B64A64Sfloat;
	}

	SK_ERROR("Unsupported gl_type specified: {0}.", gl_type);
	return vk::Format(0);
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

vk::IndexType EnumInfo::indexType(IndexType index_type)
{
	using enum vk::IndexType;
	switch (index_type)
	{
	case IndexType::UINT16: return eUint16;
	case IndexType::UINT32: return eUint32;
	}

	SK_ERROR("Unsupported index type specified: {0}.", index_type);
	return vk::IndexType(0);
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

std::string EnumInfo::stringifyResult(vk::Result result)
{
	using enum vk::Result;
	switch (result)
	{
		case eSuccess:
			return "Success";
		case eNotReady:
			return "A fence or query has not yet completed";
		case eTimeout:
			return "A wait operation has not completed in the specified time";
		case eEventSet:
			return "An event is signaled";
		case eEventReset:
			return "An event is unsignaled";
		case eIncomplete:
			return "A return array was too small for the result";
		case eErrorOutOfHostMemory:
			return "Out of host memory";
		case eErrorOutOfDeviceMemory:
			return "Out of device memory";
		case eErrorInitializationFailed:
			return "Initialization of an object could not be completed for implementation-specific reasons";
		case eErrorDeviceLost:
			return "The logical or physical device has been lost";
		case eErrorMemoryMapFailed:
			return "Mapping of a memory object has failed";
		case eErrorLayerNotPresent:
			return "A requested layer is not present or could not be loaded";
		case eErrorExtensionNotPresent:
			return "A requested extension is not supported";
		case eErrorFeatureNotPresent:
			return "A requested feature is not supported";
		case eErrorIncompatibleDriver:
			return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
		case eErrorTooManyObjects:
			return "Too many objects of the type have already been created";
		case eErrorFormatNotSupported:
			return "A requested format is not supported on this device";
		case eErrorSurfaceLostKHR:
			return "A surface is no longer available";
		case eErrorOutOfPoolMemory:
			return "A allocation failed due to having no more space in the descriptor pool";
		case eSuboptimalKHR:
			return "A swapchain no longer matches the surface properties exactly, but can still be used";
		case eErrorOutOfDateKHR:
			return "A surface has changed in such a way that it is no longer compatible with the swapchain";
		case eErrorIncompatibleDisplayKHR:
			return "The display used by a swapchain does not use the same presentable image layout";
		case eErrorNativeWindowInUseKHR:
			return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
		case eErrorValidationFailedEXT:
			return "A validation layer found an error";
		default:
			return "Unknown Vulkan error";
	}
}