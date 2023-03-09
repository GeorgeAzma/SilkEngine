#include "instance.h"
#include "graphics.h"
#include <GLFW/glfw3.h>

// Custom debug callback, when error/warning... 
// occures this function is called with the error message 
// so this is just telling vulkan how to show the error 
// (or do anything with it but mostly used for displaying the error)
#ifdef SK_ENABLE_DEBUG_UTILS
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::string message = "[Vulkan] ";
    message += pCallbackData->pMessage;

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SK_TRACE(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SK_INFO(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SK_WARN(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SK_ERROR(message);
        break;
    }

    return VK_FALSE;
}

// Since debug messenger is an extension we don't have 
// a function in core vulkan to create the debug messenger 
// so we write our own
static VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;

    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

static void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func == nullptr)
        return;

    func(instance, debugMessenger, pAllocator);
}
#endif

Instance::Instance(std::string_view app_name)
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = Graphics::apiVersion(Graphics::API_VERSION);
    app_info.pApplicationName = app_name.data();
    app_info.pEngineName = ENGINE_NAME;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector<const char*> required_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> required_validation_layers{};

#ifdef SK_ENABLE_DEBUG_UTILS
    required_extensions.push_back("VK_EXT_debug_utils");
    required_validation_layers = 
    {
        "VK_LAYER_KHRONOS_validation"
    };
    SK_VERIFY(checkExtensionSupport(required_extensions), "Vulkan: Required extension(s) not found");
    //SK_VERIFY(checkValidationLayerSupport(required_validation_layers), "Vulkan: Required validation layers(s) not found");
#endif

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app_info;
    ci.enabledLayerCount = required_validation_layers.size();
    ci.ppEnabledLayerNames = required_validation_layers.data();
    ci.enabledExtensionCount = required_extensions.size();
    ci.ppEnabledExtensionNames = required_extensions.data();

#ifdef SK_ENABLE_DEBUG_UTILS
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_ci{};
    debug_messenger_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_messenger_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_messenger_ci.pfnUserCallback = debugCallback;
    ci.pNext = &debug_messenger_ci;
#endif
   
    Graphics::vulkanAssert(vkCreateInstance(&ci, nullptr, &instance));

#ifdef SK_ENABLE_DEBUG_UTILS
    Graphics::vulkanAssert(vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_ci, nullptr, &debug_messenger));
#endif

    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
    physical_devices.resize(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());
}

Instance::~Instance()
{
#ifdef SK_ENABLE_DEBUG_UTILS
    vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}

void Instance::destroySurface(VkSurfaceKHR surface) const
{ 
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

bool Instance::checkExtensionSupport(const std::vector<const char *> &required_extensions) const
{
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());
    for (auto required_extension : required_extensions)
    {
        bool extension_found = false;
        for (const VkExtensionProperties& available_extension : available_extensions)
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

bool Instance::checkValidationLayerSupport(const std::vector<const char *> &required_layers) const
{
#ifdef SK_ENABLE_DEBUG_UTILS
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(available_layer_count);
    vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());
    for (const char *required_layer : required_layers)
    {
        bool layer_found = false;
        for (const VkLayerProperties& available_layer : available_layers)
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
#endif
    return true;
}
