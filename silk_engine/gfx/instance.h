#pragma once

class Instance : NonCopyable
{
#define SK_ENABLE_DEBUG_UTILS (VK_EXT_debug_utils && defined(SK_ENABLE_DEBUG_OUTPUT))
public:
    Instance(std::string_view app_name);
    ~Instance();

    void destroySurface(VkSurfaceKHR surface) const;

    const std::vector<VkPhysicalDevice>& getAvailablePhysicalDevices() { return physical_devices; }
    operator const VkInstance& () const { return instance; }

private:
    bool checkExtensionSupport(const std::vector<const char*>& required_extensions) const;
    bool checkValidationLayerSupport(const std::vector<const char*>& required_layers) const;

private:
    VkInstance instance = nullptr;
    std::vector<VkPhysicalDevice> physical_devices;
#ifdef SK_ENABLE_DEBUG_UTILS
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
};