#pragma once

#include "core/event.h"
#include "gfx/pipeline/render_pass.h"

class Surface;

class SwapChain : NonCopyable
{
public:
    SwapChain(const Surface& surface, VkSwapchainKHR old_swap_chain = nullptr);
    ~SwapChain();

    Image::Format getFormat() const { return image_format; }
    Image::Format getDepthFormat() const { return depth_format; }
    VkSampleCountFlagBits getSamples() const { return sample_count; }
    uint32_t getWidth() const { return extent.width; }
    uint32_t getHeight() const { return extent.height; }
    size_t getImageCount() const { return images.size(); }
    uint32_t getImageIndex() const { return image_index; }
    const std::vector<shared<Image>>& getImages() const { return images; }
    
    void recreate();

    void acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence = nullptr);
    VkResult present(VkSemaphore wait_semaphore);

    operator const VkSwapchainKHR& () const { return swap_chain; }

private:
    void create(VkSwapchainKHR old_swap_chain = nullptr);
    void destroy();

private:
    VkSwapchainKHR swap_chain = nullptr;
    std::vector<shared<Image>> images;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    uint32_t image_index = 0;
    Image::Format depth_format;
    Image::Format image_format;
    VkColorSpaceKHR color_space;
    VkSampleCountFlagBits sample_count; 
    const Surface& surface;
};