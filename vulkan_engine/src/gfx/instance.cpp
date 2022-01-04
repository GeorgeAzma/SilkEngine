#include "instance.h"

Instance::Instance()
{
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "App";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "VulkanEngine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;

    auto requiredExtensions = getRequiredExtensions();
    VE_CORE_ASSERT(checkExtensionSupport(requiredExtensions), 
        "Vulkan: Required extension(s) not found");
    create_info.enabledExtensionCount = requiredExtensions.size();
    create_info.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef VE_ENABLE_DEBUG_OUTPUT
    auto required_validation_layers = getRequiredValidationLayers();
    VE_CORE_ASSERT(checkValidationLayerSupport(required_validation_layers),
        "Vulkan: Required validation layers(s) not found");
    create_info.enabledLayerCount = required_validation_layers.size();
    create_info.ppEnabledLayerNames = required_validation_layers.data();
    debug_messenger = new DebugMessenger(&instance, &create_info);
#else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
#endif

    VE_CORE_ASSERT(vkCreateInstance(&create_info, nullptr, &instance) == VK_SUCCESS, 
        "Vulkan: Couldn't create a vulkan instance");

#ifdef VE_ENABLE_DEBUG_OUTPUT
    debug_messenger->create();
#endif
}

Instance::~Instance()
{
#ifdef VE_ENABLE_DEBUG_OUTPUT
    delete debug_messenger;
#endif
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char *> Instance::getRequiredExtensions() const
{
    uint32_t glfw_extension_count = 0;
    const char ** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    
    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef VE_ENABLE_DEBUG_OUTPUT
    extensions.push_back("VK_EXT_debug_utils");
#endif

    return extensions;
}

std::vector<VkExtensionProperties> Instance::getAvailableExtensions() const
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

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
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    return available_layers;
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
