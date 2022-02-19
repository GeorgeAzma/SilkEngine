#pragma once

#include "gfx/buffers/framebuffer.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/images/image_view.h"
#include "gfx/images/image2D.h"
#include "core/event.h"
#include "gfx/pipeline/render_pass.h"

class SwapChain : NonCopyable
{
    friend class PhysicalDevice;

public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2; //TODO: Support 2-3

public:
    SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    ~SwapChain();

    operator const VkSwapchainKHR& () const { return swap_chain; }

    const VkSurfaceFormatKHR& getSurfaceFormat() const { return surface_format; }
    const VkExtent2D& getExtent() const { return extent; }
    const std::vector<shared<Framebuffer>>& getFramebuffers() const { return framebuffers; }
    const std::vector<shared<Image2D>>& getImages() const { return images; }
    const VkRenderPass& getRenderPass() const { return *render_pass; }
    uint32_t getImageIndex() const { return image_index; }
    VkSampleCountFlagBits getSampleCount() const { return sample_count; }
    
    void recreate();

    void createFramebuffers();
    void acquireNextImage();
    void present();

    void beginFrame(size_t i = 0);
    void beginRenderPass(size_t i = 0);
    void endFrame(size_t i = 0);
    void endRenderPass();
    
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
    std::vector<VkFence> in_flight_fences = std::vector<VkFence>(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence> images_in_flight;
    std::vector<VkSemaphore> image_available_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> render_finished_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
    uint32_t current_frame = 0;
    std::vector<shared<CommandBuffer>> command_buffers;
    uint32_t image_index = 0;
    shared<Image2D> msaa_image = nullptr;
    shared<Image2D> depth = nullptr;
    VkFormat depth_format;
    shared<RenderPass> render_pass = nullptr;
    VkSampleCountFlagBits sample_count;
};