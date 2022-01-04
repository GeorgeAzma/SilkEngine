#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class SwapChain
{
public:
	SwapChain(const VkPhysicalDevice* physical_device, const VkSurfaceKHR* surface, GLFWwindow* window, const VkDevice* logical_device);
    ~SwapChain();

    const VkSwapchainKHR& getSwapChain() const { return swap_chain; }
    
    const SwapChainSupportDetails& getSwapChainSupportDetails() const { return support_details; }
    
private:
    static SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    void chooseSwapChainSurfaceFormat();
    void chooseSwapChainPresentMode();
    void chooseSwapChainExtent();
    int rateSwapChainPresentMode(VkPresentModeKHR present_mode) const;
    int rateSwapChainSurfaceFormat(VkSurfaceFormatKHR format) const;

private:
    const VkPhysicalDevice* physical_device = nullptr;
    const VkSurfaceKHR* surface = nullptr;
    GLFWwindow* window = nullptr;
    SwapChainSupportDetails support_details;
    VkSwapchainKHR swap_chain;
    const VkDevice* logical_device = nullptr;
    std::vector<VkImage> images;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    std::vector<VkImageView> image_views;

    friend class PhysicalDevice;
};