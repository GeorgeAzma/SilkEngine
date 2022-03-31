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

    const VkSurfaceFormatKHR& getSurfaceFormat() const { return surface_format; }
    const VkExtent2D& getExtent() const { return extent; }
    const std::vector<shared<Framebuffer>>& getFramebuffers() const { return framebuffers; }
    const std::vector<shared<Image2D>>& getImages() const { return images; }
    shared<RenderPass> getRenderPass() const { return render_pass; }
    uint32_t getImageIndex() const { return image_index; }
    shared<Image2D> getActiveImage() const { return images[image_index]; }
    shared<Framebuffer> getActiveFramebuffer() const { return framebuffers[image_index]; }
    VkSampleCountFlagBits getSampleCount() const { return sample_count; }
    
    void recreate();

    void createFramebuffers();
    void acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence = VK_NULL_HANDLE);
    VkResult present(VkSemaphore wait_semaphore);
    
private:
    void create(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    void destroy();

private:
    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    std::vector<shared<Image2D>> images;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    std::vector<shared<Framebuffer>> framebuffers;
    uint32_t image_index = 0;
    shared<Image2D> msaa_image = nullptr;
    shared<Image2D> depth = nullptr;
    VkFormat depth_format;
    shared<RenderPass> render_pass = nullptr;
    VkSampleCountFlagBits sample_count;
};