#pragma once

class Instance : NonCopyable
{
    static constexpr const char* app_name = "Sandbox";

public:
    Instance();
    ~Instance();

    void destroySurface(VkSurfaceKHR surface) const;

    const std::vector<VkPhysicalDevice>& getAvailablePhysicalDevices() { return physical_devices; }
    operator const VkInstance& () const { return instance; }

private:
    bool checkExtensionSupport(const std::vector<const char*>& required_extensions) const;
    bool checkValidationLayerSupport(const std::vector<const char*>& required_layers) const;

private:
    VkInstance instance = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> physical_devices; 
    VkDebugUtilsMessengerEXT debug_messenger;
};