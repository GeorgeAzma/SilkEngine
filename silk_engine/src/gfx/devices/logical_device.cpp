#include "logical_device.h"
#include "physical_device.h"
#include "gfx/graphics.h"

LogicalDevice::LogicalDevice()
{
	const auto& queue_family_indices = Graphics::physical_device->getQueueFamilyIndices();

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

	// Specifies which device features we want by enabling them
	VkPhysicalDeviceFeatures device_features{};
	device_features.samplerAnisotropy = VK_TRUE;
	device_features.occlusionQueryPrecise = VK_TRUE;
	device_features.multiDrawIndirect = VK_TRUE;

	VkPhysicalDeviceVulkan12Features vulkan_12_device_features{};
	vulkan_12_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vulkan_12_device_features.hostQueryReset = VK_TRUE;
	vulkan_12_device_features.drawIndirectCount = VK_TRUE;

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = queue_create_infos.size();
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.pEnabledFeatures = &device_features;
	auto required_extensions = getRequiredLogicalDeviceExtensions();
	create_info.enabledExtensionCount = required_extensions.size();
	create_info.ppEnabledExtensionNames = required_extensions.data();
	create_info.pNext = &vulkan_12_device_features;

	Graphics::vulkanAssert(vkCreateDevice(*Graphics::physical_device, &create_info, nullptr, &logical_device));

	//Get handles of the requried queues
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
