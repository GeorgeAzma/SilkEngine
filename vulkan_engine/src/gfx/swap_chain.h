#pragma once

#include "framebuffer.h"

class SwapChain : NonCopyable
{
public:
    SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    ~SwapChain();

    operator const VkSwapchainKHR& () const { return swap_chain; }
    const VkSurfaceFormatKHR& getSurfaceFormat() const { return surface_format; }
    const VkExtent2D& getExtent() const { return extent; }
    const std::vector<Framebuffer*>& getFramebuffers() const { return framebuffers; }
    const std::vector<VkImage>& getImages() const { return images; }
    void createFramebuffers();
    
private:
    void chooseSwapChainSurfaceFormat();
    void chooseSwapChainPresentMode();
    void chooseSwapChainExtent();
    int rateSwapChainPresentMode(VkPresentModeKHR present_mode) const;
    int rateSwapChainSurfaceFormat(VkSurfaceFormatKHR format) const;

private:
    GLFWwindow* window;
    VkSwapchainKHR swap_chain;
    std::vector<VkImage> images;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    std::vector<VkImageView> image_views;
    std::vector<Framebuffer*> framebuffers;

    friend class PhysicalDevice;
};