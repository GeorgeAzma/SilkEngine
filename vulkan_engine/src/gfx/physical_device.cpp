#include "physical_device.h"
#include "logical_device.h"
#include "queue_family.h"

PhysicalDevice::PhysicalDevice(VkInstance* instance, VkSurfaceKHR* surface)
	: instance{instance}, surface{surface}
{
	auto physical_devices = getAvailablePhysicalDevices();
	chooseMostSuitablePhysicalDevice(physical_devices);
}

PhysicalDevice::~PhysicalDevice()
{
}

std::vector<VkPhysicalDevice> PhysicalDevice::getAvailablePhysicalDevices() const
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(*instance, &device_count, nullptr);

	VE_CORE_ASSERT(device_count > 0, 
		"Vulkan: Couldn't find GPU with vulkan support");

	std::vector<VkPhysicalDevice> physical_devices(device_count);
	vkEnumeratePhysicalDevices(*instance, &device_count, physical_devices.data());

	return physical_devices;
}

void PhysicalDevice::chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices)
{
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : physical_devices)
	{
		int score = ratePhysicalDevice(device);
		if(score >= 0)
			candidates.insert(std::make_pair(score, device));
	}


	if (candidates.rbegin()->first >= 0)
	{
		physical_device = candidates.rbegin()->second;
		if(&physical_device != queue_family.getPhysicalDevice())
			queue_family = QueueFamily(&physical_device, surface);
	}

	VE_CORE_ASSERT(candidates.rbegin()->first >= 0, 
		"Vulkan: Couldn't find suitable vulkan GPU");
}

int PhysicalDevice::ratePhysicalDevice(VkPhysicalDevice physical_device)
{
	int score = 0;

	queue_family = QueueFamily(&physical_device, surface);
	if (!queue_family) return -1;
	if (!checkPhysicalDeviceExtensionSupport(LogicalDevice::getRequiredLogicalDeviceExtensions(), physical_device)) return -1;

	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	bool has_geometry_shader = physical_device_features.geometryShader;
	bool is_discrete = physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	score += is_discrete * 1000;
	score += has_geometry_shader * 250;
	score += physical_device_features.tessellationShader * 150;
	score += physical_device_features.multiViewport * 100;
	score += physical_device_features.wideLines * 50;
	score += physical_device_features.largePoints * 50;
	score += physical_device_features.occlusionQueryPrecise * 25;

	return score;
}

std::vector<VkExtensionProperties> PhysicalDevice::getAvailablePhysicalDeviceExtensions(VkPhysicalDevice physical_device) const
{
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

	return available_extensions;
}

bool PhysicalDevice::checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device) const
{
	auto available_extensions = getAvailablePhysicalDeviceExtensions(physical_device);

	for (const auto& required_extension : required_extensions)
	{
		bool extension_found = false;
		for (const auto& available_extension : available_extensions)
		{
			if (strcmp(required_extension, available_extension.extensionName) == 0)
			{
				extension_found = true;
			}
		}
		if (!extension_found)
		{
			return false;
		}
	}
	return true;
}