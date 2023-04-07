#include "instance.h"
#include "render_context.h"
#include "debug_messenger.h"


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
    return uint32_t(minimum_version) < uint32_t(vulkan_version);
}

void Instance::destroySurface(VkSurfaceKHR surface) const
{ 
    vkDestroySurfaceKHR(instance, surface, nullptr);
}