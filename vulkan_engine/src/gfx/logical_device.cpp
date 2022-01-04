#include "logical_device.h"
#include "physical_device.h"

LogicalDevice::LogicalDevice(const VkPhysicalDevice* physical_device, const VkSurfaceKHR* surface)
	: physical_device{physical_device}, surface{surface}
{
	auto queue_family_indices = PhysicalDevice::findQueueFamilies(*physical_device, *surface);
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::vector<uint32_t> queue_families = queue_family_indices.getIndices();
	std::set<uint32_t> unique_queue_families(queue_families.begin(), queue_families.end());

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family;
		queue_create_info.queueCount = 1; 
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.emplace_back(std::move(queue_create_info));
	}

	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = queue_create_infos.size();
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.pEnabledFeatures = &device_features;
	auto required_extensions = getRequiredLogicalDeviceExtensions();
	create_info.enabledExtensionCount = required_extensions.size();
	create_info.ppEnabledExtensionNames = required_extensions.data();

	VE_CORE_ASSERT(vkCreateDevice(*physical_device, &create_info, nullptr, &logical_device) == VK_SUCCESS,
		"Vulkan: Couldn't create logical device");

	vkGetDeviceQueue(logical_device, *queue_family_indices.graphics, 0, &graphics_queue);
	vkGetDeviceQueue(logical_device, *queue_family_indices.present, 0, &present_queue);
}

LogicalDevice::~LogicalDevice()
{
	vkDestroyDevice(logical_device, nullptr);
}

std::vector<const char*> LogicalDevice::getRequiredLogicalDeviceExtensions()
{
	return std::vector<const char*>{"VK_KHR_swapchain"};
}
