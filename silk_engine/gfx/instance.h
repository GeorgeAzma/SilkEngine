#pragma once

enum class VulkanVersion : uint32_t
{
    VULKAN_1_0 = VK_API_VERSION_1_0,
    VULKAN_1_1 = VK_API_VERSION_1_1,
    VULKAN_1_2 = VK_API_VERSION_1_2,
    VULKAN_1_3 = VK_API_VERSION_1_3,
};

class DebugMessenger;

class Instance : NonCopyable
{
public:
    static constexpr VulkanVersion MINIMUM_VULKAN_VERSION = VulkanVersion::VULKAN_1_1;

public:
    Instance(std::string_view app_name);
    ~Instance();

    bool checkVulkanVersionSupport(VulkanVersion minimum_version) const;

    void destroySurface(VkSurfaceKHR surface) const;

    const std::vector<VkPhysicalDevice>& getAvailablePhysicalDevices() const { return physical_devices; }
    VulkanVersion getVulkanVersion() const { return vulkan_version; }
    operator const VkInstance& () const { return instance; }

private:
#ifdef SK_ENABLE_DEBUG_MESSENGER
    DebugMessenger* debug_messenger = nullptr;
#endif
    VkInstance instance = nullptr;
    VulkanVersion vulkan_version = VulkanVersion(0);
    std::vector<VkPhysicalDevice> physical_devices; 
    std::unordered_map<std::string_view, uint32_t> supported_extensions; // extension name | extension spec version
};