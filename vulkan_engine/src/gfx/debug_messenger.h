#pragma once

class DebugMessenger
{
public:
    DebugMessenger(VkInstance instance);

private:
    VkDebugUtilsMessengerEXT debugMessenger;
};