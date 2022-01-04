#pragma once

class DebugMessenger
{
public:
    DebugMessenger(VkInstance* instance, VkInstanceCreateInfo* instance_create_info = nullptr);
    void create();
    ~DebugMessenger();

private:
    VkInstance* instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkDebugUtilsMessengerCreateInfoEXT create_info;
};