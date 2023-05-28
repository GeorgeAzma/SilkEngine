#include "instance.h"
#include "render_context.h"
#include "debug_messenger.h"
#include "silk_engine/gfx/devices/physical_device.h"
#include "silk_engine/gfx/devices/logical_device.h"

Instance::Instance(std::string_view app_name)
{
#ifdef SK_ENABLE_DEBUG_MESSENGER
    debug_messenger = new DebugMessenger();
#endif

    vkEnumerateInstanceVersion((uint32_t*)&vulkan_version);
    vulkan_version = VulkanVersion(VK_MAKE_API_VERSION(VK_API_VERSION_VARIANT(vulkan_version), VK_API_VERSION_MAJOR(vulkan_version), VK_API_VERSION_MINOR(vulkan_version), 0)); // Removes patch

    SK_ASSERT(checkVulkanVersionSupport(MINIMUM_VULKAN_VERSION), "Incompatible vulkan driver version: [{}.{}.{}] You need atleast {}.{}.{}"
        , VK_API_VERSION_MAJOR(vulkan_version), VK_API_VERSION_MINOR(vulkan_version), VK_API_VERSION_PATCH(vulkan_version)
        , VK_API_VERSION_MAJOR(MINIMUM_VULKAN_VERSION), VK_API_VERSION_MINOR(MINIMUM_VULKAN_VERSION), VK_API_VERSION_PATCH(MINIMUM_VULKAN_VERSION));

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = uint32_t(vulkan_version);
    app_info.pApplicationName = app_name.data();
    app_info.pEngineName = ENGINE_NAME;

    uint32_t supported_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr);
    std::vector<VkExtensionProperties> supported_extensions(supported_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions.data());
    for (const auto& extension : supported_extensions)
        this->supported_extensions.emplace(extension.extensionName, extension.specVersion);

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector<const char*> required_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> required_validation_layers{};

#ifdef SK_ENABLE_DEBUG_MESSENGER
    required_extensions.push_back("VK_EXT_debug_utils");
    for (const auto& required_extension : required_extensions)
        if (!this->supported_extensions.contains(required_extension))
            SK_ERROR("Vulkan: Required extension({}) not found", required_extension);

    required_validation_layers = debug_messenger->getRequiredValidationLayers();
#endif

    VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = countof(enables);
    features.pEnabledValidationFeatures = enables;

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app_info;
    ci.enabledLayerCount = required_validation_layers.size();
    ci.ppEnabledLayerNames = required_validation_layers.data();
    ci.enabledExtensionCount = required_extensions.size();
    ci.ppEnabledExtensionNames = required_extensions.data();
    ci.pNext = &features;

#ifdef SK_ENABLE_DEBUG_MESSENGER
    features.pNext = &debug_messenger->getCreateInfo();
#endif
   
    RenderContext::vulkanAssert(vkCreateInstance(&ci, nullptr, &instance));

    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
    physical_devices.resize(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

#ifdef SK_ENABLE_DEBUG_MESSENGER
    debug_messenger->init(instance);
#endif
}

Instance::~Instance()
{
#ifdef SK_ENABLE_DEBUG_MESSENGER
    delete debug_messenger;
#endif
    vkDestroyInstance(instance, nullptr);
}

bool Instance::checkVulkanVersionSupport(VulkanVersion minimum_version) const
{
    return ecast(minimum_version) <= ecast(vulkan_version);
}

void Instance::destroySurface(VkSurfaceKHR surface) const
{ 
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

PhysicalDevice* Instance::selectPhysicalDevice(VkSurfaceKHR surface) const
{
	int max_score = -1;
	unique<PhysicalDevice> best_physical_device = nullptr;
	for (const auto& vk_physical_device : getAvailablePhysicalDevices())
	{
		int score = -1;
		unique<PhysicalDevice> physical_device = makeUnique<PhysicalDevice>(*this, vk_physical_device);

		if (physical_device->getGraphicsQueue() == -1)
			continue;

		bool supports_required_extensions = true;
		for (const auto& required_extension : LogicalDevice::getRequiredExtensions())
			if (!physical_device->supportsExtension(required_extension))
			{
				supports_required_extensions = false;
				break;
			}
		if (!supports_required_extensions)
			continue;

		for (const auto& preferred_extension : LogicalDevice::getPreferredExtensions())
			if (physical_device->supportsExtension(preferred_extension))
				score += 100;

		// TODO:
		if (surface)
		{
			bool supports_surface = true;
			if (!supports_surface)
				continue;
		}

		score = 0;

		const VkPhysicalDeviceProperties& properties = physical_device->getProperties();
		const VkPhysicalDeviceFeatures& features = physical_device->getFeatures();

		score += features.multiDrawIndirect * 1200;
		score += features.samplerAnisotropy * 900;
		score += (physical_device->getComputeQueue() != -1) * 700;
		score += (physical_device->getTransferQueue() != -1) * 300;
		score += (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 500;
		score += features.multiDrawIndirect * 250;
		score += features.geometryShader * 200;
		score += features.tessellationShader * 100;
		score += features.samplerAnisotropy * 80;
		score += features.fragmentStoresAndAtomics * 50;
		score += features.vertexPipelineStoresAndAtomics * 30;
		score += features.shaderSampledImageArrayDynamicIndexing * 25;
		score += features.shaderStorageBufferArrayDynamicIndexing * 25;
		score += features.shaderStorageImageArrayDynamicIndexing * 25;
		score += features.shaderUniformBufferArrayDynamicIndexing * 25;
		score += features.occlusionQueryPrecise * 25;
		score += features.multiViewport * 25;
		score += features.wideLines * 20;
		score += features.largePoints * 10;
		score += features.imageCubeArray * 10;
		score += features.pipelineStatisticsQuery * 5;
		score += features.shaderFloat64 * 5;
		score += features.alphaToOne;
		score += features.depthBiasClamp;
		score += features.depthBounds;
		score += features.depthClamp;
		score += features.drawIndirectFirstInstance;
		score += features.dualSrcBlend;
		score += features.fillModeNonSolid;
		score += features.fullDrawIndexUint32;
		score += features.independentBlend;
		score += features.inheritedQueries;
		score += features.logicOp;
		score += features.robustBufferAccess;
		score += features.sampleRateShading;
		score += features.shaderClipDistance;
		score += features.shaderCullDistance;
		score += features.shaderImageGatherExtended;
		score += features.shaderInt16;
		score += features.shaderInt64;
		score += features.shaderResourceMinLod;
		score += features.shaderResourceResidency;
		score += features.shaderStorageImageExtendedFormats;
		score += features.shaderStorageImageMultisample;
		score += features.shaderStorageImageReadWithoutFormat;
		score += features.shaderStorageImageWriteWithoutFormat;
		score += features.shaderTessellationAndGeometryPointSize;
		score += features.sparseBinding;
		score += features.sparseResidency16Samples;
		score += features.sparseResidency2Samples;
		score += features.sparseResidency4Samples;
		score += features.sparseResidency8Samples;
		score += features.sparseResidency16Samples;
		score += features.sparseResidencyAliased;
		score += features.sparseResidencyBuffer;
		score += features.sparseResidencyImage2D;
		score += features.sparseResidencyImage3D;
		score += features.textureCompressionASTC_LDR;
		score += features.textureCompressionBC;
		score += features.textureCompressionETC2;
		score += features.variableMultisampleRate;

		if (score > max_score)
		{
			max_score = score;
			best_physical_device = std::move(physical_device);
		}
	}

#ifdef SK_ENABLE_DEBUG_OUTPUT
	if (best_physical_device)
	{
		std::string device_type = "Other";
		switch (best_physical_device->getProperties().deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: device_type = "Integrated GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: device_type = "Discrete GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: device_type = "Virtual GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: device_type = "CPU"; break;
		}
		SK_INFO("{}: {}", device_type, best_physical_device->getProperties().deviceName);
	}
#endif

	return best_physical_device.release();
}