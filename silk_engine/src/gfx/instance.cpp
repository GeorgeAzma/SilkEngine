#include "instance.h"
#include "graphics.h"

// Custom debug callback, when error/warning... 
// occures this function is called with the error message 
// so this is just telling vulkan how to show the error 
// (or do anything with it but mostly used for displaying the error)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::string message = "[Vulkan]";

    message += ' ';
    message += pCallbackData->pMessage;
    message += '\n';


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

Instance::Instance()
{
    vk::ApplicationInfo app_info(app_name, 1, engine_name, 1, EnumInfo::apiVersion(Graphics::API_VERSION));

    //Get required extensions and see if they are supported

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector<const char*> required_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> required_validation_layers{};

#ifdef SK_ENABLE_DEBUG_OUTPUT
    required_extensions.push_back("VK_EXT_debug_utils");

    required_validation_layers = 
    {
        "VK_LAYER_KHRONOS_validation"
    };
    SK_ASSERT(checkExtensionSupport(required_extensions),
        "Vulkan: Required extension(s) not found");

    SK_ASSERT(checkValidationLayerSupport(required_validation_layers),
        "Vulkan: Required validation layers(s) not found");
    
    using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
    vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_ci{};
    debug_messenger_ci.messageSeverity = eError | eWarning | eInfo | eVerbose;
    debug_messenger_ci.messageType = eGeneral | eValidation | ePerformance;
    debug_messenger_ci.pfnUserCallback = debugCallback;
#endif

    vk::InstanceCreateInfo ci({}, &app_info, required_validation_layers, required_extensions);

#ifdef SK_ENABLE_DEBUG_OUTPUT
    ci.setPNext(&debug_messenger_ci);
#endif
   
    instance = vk::createInstance(ci);

#ifdef SK_ENABLE_DEBUG_OUTPUT
    debug_messenger = instance.createDebugUtilsMessengerEXT(debug_messenger_ci);
#endif

    physical_devices = instance.enumeratePhysicalDevices();
}

Instance::~Instance()
{
#ifdef SK_ENABLE_DEBUG_OUTPUT
    instance.destroyDebugUtilsMessengerEXT(debug_messenger);
#endif
    instance.destroy();
}

bool Instance::checkExtensionSupport(const std::vector<const char *> &required_extensions) const
{
    std::vector<vk::ExtensionProperties> available_extensions = vk::enumerateInstanceExtensionProperties();
    for (const char *required_extension : required_extensions)
    {
        bool extension_found = false;
        for (vk::ExtensionProperties available_extension : available_extensions)
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
#ifdef SK_ENABLE_DEBUG_OUTPUT
    std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();
    for (const char *required_layer : required_layers)
    {
        bool layer_found = false;
        for (vk::LayerProperties available_layer : available_layers)
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
