#pragma once

class DebugMessenger
{
public:
    DebugMessenger();

private:
    VkDebugUtilsMessengerEXT debugMessenger;
};