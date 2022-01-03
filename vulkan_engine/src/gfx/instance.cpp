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
    VE_CORE_ASSERT(checkExtensionSupport(requiredExtensions), "Vulkan Error: Required extension(s) not found");
    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef VE_ENABLE_DEBUG_OUTPUT
    auto requiredValidationLayers = getRequiredValidationLayers();
    VE_CORE_ASSERT(checkValidationLayerSupport(requiredValidationLayers), "Vulkan Error: Required validation layers(s) not found");
    createInfo.enabledLayerCount = requiredValidationLayers.size();
    createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
#endif

    VE_CORE_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance), "Vulkan Error: Couldn't create a vulkan instance");
}

Instance::~Instance()
{
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char *> Instance::getRequiredExtensions() const
{
    std::vector<const char *> extensions;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (size_t i = 0; i < glfwExtensionCount; ++i)
        extensions.emplace_back(glfwExtensions[i]);

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

bool Instance::checkExtensionSupport(const std::vector<const char*>& required_extensions) const
{
    std::vector<VkExtensionProperties> available_extensions = getAvailableExtensions();
    for (const char* required_extension : required_extensions)
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

std::vector<const char*> Instance::getRequiredValidationLayers() const
{
    return std::vector<const char*>{"VK_LAYER_KHRONOS_validation"};
}

std::vector<VkLayerProperties> Instance::getAvailableValidationLayers() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

bool Instance::checkValidationLayerSupport(const std::vector<const char*>& required_layers) const
{
    std::vector<VkLayerProperties> available_layers = getAvailableValidationLayers();
    for (const char* required_layer : required_layers)
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    std::string message = "Vulkan Error: ";
    switch (messageType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        message += "[General]";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        message += "[Performance]";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        message += "[Validation]";
        break;
    default:
        message += "[Unknown]";
        break;
    }

    message += pCallbackData->pMessage;

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        VE_CORE_TRACE(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        VE_CORE_INFO(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        VE_CORE_WARN(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        VE_CORE_ERROR(message);
        break;
    }

    return VK_FALSE;
}