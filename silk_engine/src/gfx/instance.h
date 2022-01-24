#pragma once
#include "debug_messenger.h"

class Instance : NonCopyable
{
public:
    Instance();
    ~Instance();

    const std::vector<VkPhysicalDevice>& getAvailablePhysicalDevices() { return physical_devices; }

    operator const VkInstance& () const { return instance; }
private:
    std::vector<const char *> getRequiredExtensions() const;
    std::vector<VkExtensionProperties> getAvailableExtensions() const;
    bool checkExtensionSupport(const std::vector<const char*>& required_extensions) const;
    std::vector<const char *> getRequiredValidationLayers() const;
    std::vector<VkLayerProperties> getAvailableValidationLayers() const;
    bool checkValidationLayerSupport(const std::vector<const char*>& required_layers) const;
    void findAvailablePhysicalDevices();

private:
    VkInstance instance;
    DebugMessenger* debug_messenger = nullptr;
    std::vector<VkPhysicalDevice> physical_devices;
};