#pragma once

#ifdef SK_ENABLE_DEBUG_MESSENGER
class DebugMessenger
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static VkResult create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
public:
    DebugMessenger();
    ~DebugMessenger();

    void init();

    const VkDebugUtilsMessengerCreateInfoEXT& getCreateInfo() const { return ci; }
    const std::vector<const char*>& getRequiredValidationLayers() const { return required_validation_layers; }

private:
    VkDebugUtilsMessengerEXT debug_messenger; 
    VkDebugUtilsMessengerCreateInfoEXT ci = {};
    std::unordered_map<std::string_view, VkLayerProperties> available_validation_layers; // layer name | layer properties
    std::vector<const char*> required_validation_layers;
};
#else 
class DebugMessenger {};
#endif