#include "queue_family.h"

QueueFamily::QueueFamily(VkPhysicalDevice* physical_device, VkSurfaceKHR* surface) 
	: physical_device{physical_device}, surface{surface}
{
	findQueueFamilies();
}

bool QueueFamily::findQueueFamilies()
{
	if (indices.isSuitable()) 
		return true;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, queue_families.data());

	for (size_t i = 0; i < queue_family_count; ++i)
	{
		const auto& queue_family = queue_families[i];
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphics = i;

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(*physical_device, i, *surface, &present_support);
		if (present_support)
			indices.present = i;

		if (indices.isSuitable()) 
			break;
	}


	return indices.isSuitable();
}
