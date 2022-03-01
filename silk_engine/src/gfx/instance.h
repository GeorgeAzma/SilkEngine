#pragma once

class Instance : NonCopyable
{
    static constexpr const char* app_name = "Sandbox";
    static constexpr const char* engine_name = "Silk Engine";
public:
    Instance();
    ~Instance();

    void destroySurface(vk::SurfaceKHR surface) const { instance.destroySurfaceKHR(surface); }

    const std::vector<vk::PhysicalDevice>& getAvailablePhysicalDevices() { return physical_devices; }
    operator const vk::Instance& () const { return instance; }

private:
    bool checkExtensionSupport(const std::vector<const char*>& required_extensions) const;
    bool checkValidationLayerSupport(const std::vector<const char*>& required_layers) const;

private:
    vk::Instance instance;
    std::vector<vk::PhysicalDevice> physical_devices; 
    vk::DebugUtilsMessengerEXT debug_messenger;
};