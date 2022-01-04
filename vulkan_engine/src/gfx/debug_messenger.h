#pragma once

class DebugMessenger
{
public:
    DebugMessenger(VkInstance* instance);
    ~DebugMessenger();
    const VkDebugUtilsMessengerCreateInfoEXT& getCreateInfo() const { return create_info; }
    void create();

private:
    VkInstance* instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkDebugUtilsMessengerCreateInfoEXT create_info;
};