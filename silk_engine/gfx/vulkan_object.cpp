#ifdef SK_ENABLE_DEBUG_OUTPUT

#include "vulkan_object.h"
#include "render_context.h"
#include "devices/logical_device.h"
#include "instance.h"

void VulkanObject::setName(Type type, uint64_t handle, const char* name)
{
	if (type == Type::UNKNOWN || handle == 0 || name == nullptr)
		return;

	static auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(RenderContext::getInstance(), "vkSetDebugUtilsObjectNameEXT");
	if (vkSetDebugUtilsObjectNameEXT == nullptr) // VK_ERROR_EXTENSION_NOT_PRESENT
		return;

	VkDebugUtilsObjectNameInfoEXT name_info = {};
	name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	name_info.objectType = VkObjectType(type);
	name_info.objectHandle = handle;
	name_info.pObjectName = name;

	vkSetDebugUtilsObjectNameEXT(RenderContext::getLogicalDevice(), &name_info);
}

#endif