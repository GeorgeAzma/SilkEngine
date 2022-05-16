#pragma once

#include "gfx/buffers/framebuffer.h"
#include "gfx/images/image_view.h"
#include "gfx/images/image2D.h"
#include "core/event.h"
#include "gfx/pipeline/render_pass.h"

class SwapChain : NonCopyable
{
public:
    SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    ~SwapChain();

    operator const VkSwapchainKHR& () const { return swap_chain; }

    ImageFormat getFormat() const { return image_format; }
    ImageFormat getDepthFormat() const { return depth_format; }
    VkSampleCountFlagBits getSamples() const { return sample_count; }
    uint32_t getWidth() const { return extent.width; }
    uint32_t getHeight() const { return extent.height; }
    size_t getImageCount() const { return images.size(); }
    shared<RenderPass> getRenderPass() const { return render_pass; }
    uint32_t getImageIndex() const { return image_index; }
    shared<Image2D> getActiveImage() const { return images[image_index]; }
    shared<Framebuffer> getActiveFramebuffer() const { return framebuffers[image_index]; }
    
    void recreate();

    void acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence = VK_NULL_HANDLE);
    VkResult present(VkSemaphore wait_semaphore);
    
private:
    void create(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    void destroy();

private:
    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    std::vector<shared<Image2D>> images;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    std::vector<shared<Framebuffer>> framebuffers;
    uint32_t image_index = 0;
    ImageFormat depth_format;
    ImageFormat image_format;
    VkColorSpaceKHR color_space;
    shared<RenderPass> render_pass = nullptr;
    VkSampleCountFlagBits sample_count;
};