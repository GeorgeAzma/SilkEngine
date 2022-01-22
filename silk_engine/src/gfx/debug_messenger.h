#pragma once

class DebugMessenger : NonCopyable
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