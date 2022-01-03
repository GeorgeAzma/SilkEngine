#include "instance.h"

Instance::Instance(const AppInfo &app_info)
{
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = app_info.name;
    application_info.applicationVersion = app_info.version;
    application_info.pEngineName = "VulkanEngine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &application_info;

    auto requiredExtensions = getRequiredExtensions();
    VE_CORE_ASSERT(checkExtensionSupport(requiredExtensions), 
        "Vulkan: Required extension(s) not found");
    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef VE_ENABLE_DEBUG_OUTPUT
    auto requiredValidationLayers = getRequiredValidationLayers();
    VE_CORE_ASSERT(checkValidationLayerSupport(requiredValidationLayers), 
        "Vulkan: Required validation layers(s) not found");
    createInfo.enabledLayerCount = requiredValidationLayers.size();
    createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
    debugMessenger = new DebugMessenger(&instance, &createInfo);
#else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
#endif

    VE_CORE_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS, 
        "Vulkan: Couldn't create a vulkan instance");

    debugMessenger->create();
}

Instance::~Instance()
{
#ifdef VE_ENABLE_DEBUG_OUTPUT
    delete debugMessenger;
#endif
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char *> Instance::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VE_ENABLE_DEBUG_OUTPUT
    extensions.push_back("VK_EXT_debug_utils");
#endif

    return extensions;
}

std::vector<VkExtensionProperties> Instance::getAvailableExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

bool Instance::checkExtensionSupport(const std::vector<const char *> &required_extensions) const
{
    std::vector<VkExtensionProperties> available_extensions = getAvailableExtensions();
    for (const char *required_extension : required_extensions)
    {
        bool extension_found = false;
        for (VkExtensionProperties available_extension : available_extensions)
        {
            if (strcmp(available_extension.extensionName, required_extension) == 0)
            {
                extension_found = true;
                break;
            }
        }
        if (!extension_found)
            return false;
    }
    return true;
}

std::vector<const char *> Instance::getRequiredValidationLayers() const
{
    return std::vector<const char *>
    {
        "VK_LAYER_KHRONOS_validation"
    };
}

std::vector<VkLayerProperties> Instance::getAvailableValidationLayers() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

bool Instance::checkValidationLayerSupport(const std::vector<const char *> &required_layers) const
{
    std::vector<VkLayerProperties> available_layers = getAvailableValidationLayers();
    for (const char *required_layer : required_layers)
    {
        bool layer_found = false;
        for (VkLayerProperties available_layer : available_layers)
        {
            if (strcmp(available_layer.layerName, required_layer) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
            return false;
    }
    return true;
}
