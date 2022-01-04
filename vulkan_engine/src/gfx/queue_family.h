#pragma once
#include <optional>
#include <type_traits>

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool isSuitable() const 
    { 
        return graphics.has_value() && present.has_value();
    }
};

class QueueFamily
{
public:
    QueueFamily() = default;
    QueueFamily(VkPhysicalDevice* physical_device, VkSurfaceKHR* surface);

public:
    operator bool() const { return indices.isSuitable(); }
    QueueFamilyIndices getIndices() const { return indices; }
    const VkPhysicalDevice* getPhysicalDevice() const { return physical_device; }

private:
    bool findQueueFamilies();

private:
    VkPhysicalDevice* physical_device = nullptr;
    QueueFamilyIndices indices = {};
    VkSurfaceKHR* surface = nullptr;
};