#pragma once
#include <optional>
#include <type_traits>

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphics_family;

    bool isSuitable() const 
    { 
        return graphics_family.has_value();
    }
};

class QueueFamily
{
public:
    QueueFamily(VkPhysicalDevice* physical_device);

public:
    operator bool() const { return indices.isSuitable(); }

private:
    void findQueueFamilies();

private:
    VkPhysicalDevice* physical_device = nullptr;
    QueueFamilyIndices indices = {};
};