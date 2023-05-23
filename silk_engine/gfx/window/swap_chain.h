#pragma once

#include "silk_engine/core/event.h"
#include "silk_engine/gfx/images/image.h"

class Surface;

class SwapChain : NoCopy
{
public:
    SwapChain(const Surface& surface, bool vsync = false);
    ~SwapChain();

    uint32_t getWidth() const { return extent.width; }
    uint32_t getHeight() const { return extent.height; }
    size_t getImageCount() const { return images.size(); }
    uint32_t getImageIndex() const { return image_index; }
    const std::vector<shared<Image>>& getImages() const { return images; }

    void recreate(bool vsync = false);

    bool acquireNextImage(VkSemaphore signal_semaphore = nullptr, VkFence signal_fence = nullptr) const;
    bool present(VkSemaphore wait_semaphore = nullptr) const;

    operator const VkSwapchainKHR& () const { return swap_chain; }

private:
    VkSwapchainKHR swap_chain = nullptr;
    std::vector<shared<Image>> images{};
    VkPresentModeKHR present_mode{};
    VkExtent2D extent = { 0, 0 };
    mutable uint32_t image_index = 0;
    const Surface& surface;
};