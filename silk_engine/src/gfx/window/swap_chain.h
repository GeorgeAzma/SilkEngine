#pragma once

#include "gfx/buffers/framebuffer.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/images/image_view.h"
#include "gfx/images/image2D.h"
#include "core/event.h"
#include "gfx/pipeline/render_pass.h"

class SwapChain : NonCopyable
{
public:
    SwapChain(const std::optional<vk::SwapchainKHR>& old_swap_chain = {});
    ~SwapChain();

    operator const vk::SwapchainKHR& () const { return swap_chain; }

    const vk::SurfaceFormatKHR& getSurfaceFormat() const { return surface_format; }
    const vk::Extent2D& getExtent() const { return extent; }
    const std::vector<shared<Framebuffer>>& getFramebuffers() const { return framebuffers; }
    const std::vector<shared<Image2D>>& getImages() const { return images; }
    const vk::RenderPass& getRenderPass() const { return *render_pass; }
    uint32_t getImageIndex() const { return image_index; }
    shared<Image2D> getActiveImage() const { return images[image_index]; }
    vk::SampleCountFlagBits getSampleCount() const { return sample_count; }
    
    void recreate();

    void createFramebuffers();
    void acquireNextImage(vk::Semaphore signal_semaphore, vk::Fence signal_fence = VK_NULL_HANDLE);
    vk::Result present(vk::Semaphore wait_semaphore);

    void beginRenderPass();
    void endRenderPass();
    
private:
    void create(const std::optional<vk::SwapchainKHR>& old_swap_chain = {});
    void destroy();

private:
    vk::SwapchainKHR swap_chain = VK_NULL_HANDLE;
    std::vector<shared<Image2D>> images;
    vk::SurfaceFormatKHR surface_format;
    vk::PresentModeKHR present_mode;
    vk::Extent2D extent;
    std::vector<shared<Framebuffer>> framebuffers;
    uint32_t image_index = 0;
    shared<Image2D> msaa_image = nullptr;
    shared<Image2D> depth = nullptr;
    vk::Format depth_format;
    shared<RenderPass> render_pass = nullptr;
    vk::SampleCountFlagBits sample_count;
};