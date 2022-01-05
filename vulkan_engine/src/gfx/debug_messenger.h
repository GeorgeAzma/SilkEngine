#pragma once

class DebugMessenger
{
public:
    DebugMessenger();
    ~DebugMessenger();
    const VkDebugUtilsMessengerCreateInfoEXT& getCreateInfo() const { return create_info; }
    void create(VkInstance instance);

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkDebugUtilsMessengerCreateInfoEXT create_info;
};