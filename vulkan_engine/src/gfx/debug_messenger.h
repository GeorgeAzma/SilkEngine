#pragma once

class DebugMessenger
{
public:
    DebugMessenger(VkInstance* instance, VkInstanceCreateInfo* instanceCreateInfo = nullptr);
    void create();
    ~DebugMessenger();

private:
    VkInstance* instance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
};