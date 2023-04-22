#ifdef SK_ENABLE_DEBUG_MESSENGER

#include "debug_messenger.h"
#include "gfx/render_context.h"
#include "gfx/instance.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::string message = pCallbackData->pMessage;
    std::string delimiter = std::format("MessageID = 0x{:x} | ", *(const uint32_t*)&pCallbackData->messageIdNumber);
    size_t offset = message.find(delimiter);
    if (offset != std::string::npos)
        message = message.substr(offset + delimiter.size());

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

    if (size_t off = message.find("https://", offset != std::string::npos ? offset : 0); off != std::string::npos)
        message = message.substr(0, off);

    if (pCallbackData->objectCount)
    {
        message += " [";
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
        {
            bool has_type = true;
            switch (pCallbackData->pObjects[i].objectType)
            {
            case VK_OBJECT_TYPE_UNKNOWN: message += "Unknown"; break;
            case VK_OBJECT_TYPE_INSTANCE: message += "Instance"; break;
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE: message += "Physical Device"; break;
            case VK_OBJECT_TYPE_DEVICE: message += "Device"; break;
            case VK_OBJECT_TYPE_QUEUE: message += "Queue"; break;
            case VK_OBJECT_TYPE_SEMAPHORE: message += "Semaphore"; break;
            case VK_OBJECT_TYPE_COMMAND_BUFFER: message += "Command Buffer"; break;
            case VK_OBJECT_TYPE_FENCE: message += "Fence"; break;
            case VK_OBJECT_TYPE_DEVICE_MEMORY: message += "Memory"; break;
            case VK_OBJECT_TYPE_BUFFER: message += "Buffer"; break;
            case VK_OBJECT_TYPE_IMAGE: message += "Image"; break;
            case VK_OBJECT_TYPE_EVENT: message += "Event"; break;
            case VK_OBJECT_TYPE_QUERY_POOL: message += "Query Pool"; break;
            case VK_OBJECT_TYPE_BUFFER_VIEW: message += "Buffer View"; break;
            case VK_OBJECT_TYPE_IMAGE_VIEW: message += "Image View"; break;
            case VK_OBJECT_TYPE_SHADER_MODULE: message += "Shader Module"; break;
            case VK_OBJECT_TYPE_PIPELINE_CACHE: message += "Pipeline Cache"; break;
            case VK_OBJECT_TYPE_PIPELINE_LAYOUT: message += "Pipeline Layout"; break;
            case VK_OBJECT_TYPE_RENDER_PASS: message += "Render Pass"; break;
            case VK_OBJECT_TYPE_PIPELINE: message += "Pipeline"; break;
            case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: message += "Descriptor Set Layout"; break;
            case VK_OBJECT_TYPE_SAMPLER: message += "Sampler"; break;
            case VK_OBJECT_TYPE_DESCRIPTOR_POOL: message += "Descriptor Pool"; break;
            case VK_OBJECT_TYPE_DESCRIPTOR_SET: message += "Descriptor Set"; break;
            case VK_OBJECT_TYPE_FRAMEBUFFER: message += "Framebuffer"; break;
            case VK_OBJECT_TYPE_COMMAND_POOL: message += "Command Pool"; break;
            case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: message += "Sampler YCBR Conversion"; break;
            case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: message += "Descriptor Update Template"; break;
            case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT: message += "Private Data Slot"; break;
            case VK_OBJECT_TYPE_SURFACE_KHR: message += "Surface"; break;
            case VK_OBJECT_TYPE_SWAPCHAIN_KHR: message += "Swapchain"; break;
            case VK_OBJECT_TYPE_DISPLAY_KHR: message += "Display"; break;
            case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: message += "Display Mode"; break;
            case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: message += "Debug Report Callback"; break;
            default: has_type = false; break;
            }
            if (pCallbackData->pObjects[i].pObjectName)
            {
                message += ": ";
                message += pCallbackData->pObjects[i].pObjectName;
            }
            if ((has_type || pCallbackData->pObjects[i].pObjectName) && i < pCallbackData->objectCount - 1)
                message += ", ";
        }
    }

    return VK_FALSE;
}

VkResult DebugMessenger::create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;

    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void DebugMessenger::destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")) 
        func(instance, debugMessenger, pAllocator);
}

DebugMessenger::DebugMessenger()
{
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(available_layer_count);
    vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());
    for (const auto& layer : available_layers)
        available_validation_layers.emplace(layer.layerName, layer);

    required_validation_layers = { "VK_LAYER_KHRONOS_validation" };
    for (const auto& required_validation_layer : required_validation_layers)
        if (!available_validation_layers.contains(required_validation_layer))
            SK_ERROR("Vulkan: Required validation layer({}) not found", required_validation_layer);

    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    ci.pfnUserCallback = debugCallback;
}

void DebugMessenger::init(VkInstance instance)
{
    this->instance = instance;
    create(instance, &ci, nullptr, &debug_messenger);
}

DebugMessenger::~DebugMessenger()
{
    destroy(instance, debug_messenger, nullptr);
}
#endif