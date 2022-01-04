#include "physical_device.h"

PhysicalDevice::PhysicalDevice(VkInstance* instance)
{
}

PhysicalDevice::~PhysicalDevice()
{
}

std::vector<VkPhysicalDevice> PhysicalDevice::getAvailablePhysicalDevices() const
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(*instance, &device_count, nullptr);

	VE_CORE_ASSERT(device_count > 0, "Vulkan: Couldn't find GPU with vulkan support");

	std::vector<VkPhysicalDevice> physical_devices(device_count);
	vkEnumeratePhysicalDevices(*instance, &device_count, physical_devices.data());

	return physical_devices;
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice physical_device) const
{
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	bool has_geometry_shader = physical_device_features.geometryShader;
	bool is_discrete = physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	bool is_suitable = is_descrete && has_geometry_shader;

	return is_suitable;
}

void PhysicalDevice::chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices)
{
	for (const auto& device : physical_devices)
	{
		if (isDeviceSuitable(device)) 
		{
			physical_device = device;
			break;
		}
	}

	VK_CORE_ASSERT(physical_device != VK_NULL_HANDLE, "Vulkan: Couldn't find suitable vulkan GPU");
}
