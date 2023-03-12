#pragma once

class Instance : NonCopyable
{
public:
    Instance(std::string_view app_name);
    ~Instance();

    void destroySurface(VkSurfaceKHR surface) const;

    const std::vector<VkPhysicalDevice>& getAvailablePhysicalDevices() { return physical_devices; }
    operator const VkInstance& () const { return instance; }

private:
    VkInstance instance = nullptr;
    std::vector<VkPhysicalDevice> physical_devices; 
    std::unordered_map<std::string_view, uint32_t> available_extensions; // extension name | extension spec version
};