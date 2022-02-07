#include "debug_messenger.h"
#include "graphics.h"

// Custom debug callback, when error/warning... 
// occures this function is called with the error message 
// so this is just telling vulkan how to show the error 
// (or do anything with it but mostly used for displaying the error)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::string message = "[Vulkan]";
    //switch (messageType)
    //{
    //case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
    //    message += "[General]";
    //    break;
    //case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    //    message += "[Performance]";
    //    break;
    //case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    //    message += "[Validation]";
    //    break;
    //default:
    //    message += "[Unknown]";
    //    break;
    //}

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
static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"); 
    
    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if (func == nullptr) 
        return;
    
    func(instance, debugMessenger, pAllocator);
}

DebugMessenger::DebugMessenger()
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debugCallback;
}

void DebugMessenger::create(VkInstance instance)
{
    this->instance = instance;
    Graphics::vulkanAssert(createDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger));
}

DebugMessenger::~DebugMessenger()
{
    destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
}
