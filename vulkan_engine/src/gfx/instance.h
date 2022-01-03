#pragma once
struct AppInfo
{
    const char *name = "App";
    unsigned int version = 0;
};

class Instance
{
public:
    Instance(const AppInfo &app_info = {});
    ~Instance();

private:
    std::vector<const char *> getRequiredExtensions() const;
    std::vector<VkExtensionProperties> getAvailableExtensions() const;
    bool checkExtensionSupport(const std::vector<const char*>& required_extensions) const;
    std::vector<const char *> getRequiredValidationLayers() const;
    std::vector<VkLayerProperties> getAvailableValidationLayers() const;
    bool checkValidationLayerSupport(const std::vector<const char*>& required_layers) const;

private:
    VkInstance instance;
};