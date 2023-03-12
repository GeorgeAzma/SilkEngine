#include "instance.h"
#include "graphics.h"
#include "debug_messenger.h"
#include <GLFW/glfw3.h>


Instance::Instance(std::string_view app_name)
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = uint32_t(Graphics::API_VERSION);
    app_info.pApplicationName = app_name.data();
    app_info.pEngineName = ENGINE_NAME;

    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());
    for (const auto& extension : available_extensions)
        this->available_extensions.emplace(extension.extensionName, extension.specVersion);

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector<const char*> required_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> required_validation_layers{};

#ifdef SK_ENABLE_DEBUG_MESSENGER
    required_extensions.push_back("VK_EXT_debug_utils");
    for (const auto& required_extension : required_extensions)
        if (!this->available_extensions.contains(required_extension))
            SK_ERROR("Vulkan: Required extension({}) not found", required_extension);

    required_validation_layers = Graphics::debug_messenger->getRequiredValidationLayers();
#endif

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app_info;
    ci.enabledLayerCount = required_validation_layers.size();
    ci.ppEnabledLayerNames = required_validation_layers.data();
    ci.enabledExtensionCount = required_extensions.size();
    ci.ppEnabledExtensionNames = required_extensions.data();
#ifdef SK_ENABLE_DEBUG_MESSENGER
    ci.pNext = &Graphics::debug_messenger->getCreateInfo();
#endif
   
    Graphics::vulkanAssert(vkCreateInstance(&ci, nullptr, &instance));

    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
    physical_devices.resize(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());
}

Instance::~Instance()
{
    vkDestroyInstance(instance, nullptr);
}

void Instance::destroySurface(VkSurfaceKHR surface) const
{ 
    vkDestroySurfaceKHR(instance, surface, nullptr);
}